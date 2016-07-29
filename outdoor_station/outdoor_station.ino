/**
  The MySensors Arduino library handles the wireless radio link and protocol
  between your home built sensors/actuators and HA controller of choice.
  The sensors forms a self healing radio network with optional repeaters. Each
  repeater and gateway builds a routing tables in EEPROM which keeps track of the
  network topology allowing messages to be routed to nodes.

  Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
  Copyright (C) 2013-2015 Sensnology AB
  Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

  Documentation: http://www.mysensors.org
  Support Forum: http://forum.mysensors.org

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  version 2 as published by the Free Software Foundation.

*******************************

  DESCRIPTION
  The RS485 Gateway prints data received from sensors on the serial link.
  The gateway accepts input on seral which will be sent out on
  the RS485 link.

  Wire connections (OPTIONAL):
  - Inclusion button should be connected between digital pin 3 and GND
  - RX/TX/ERR leds need to be connected between +5V (anode) and digital pin 6/5/4 with resistor 270-330R in a series

  LEDs (OPTIONAL):
  - To use the feature, uncomment MY_LEDS_BLINKING_FEATURE in MyConfig.h
  - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
  - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
  - ERR (red) - fast blink on error during transmission error or recieve crc error

  The gateway uses AltSoftSerial to handle two serial links
  on one Arduino. Use the following pins for RS485 link

   Board          Transmit  Receive   PWM Unusable
  -----          --------  -------   ------------
  Teensy 3.0 & 3.1  21        20         22
  Teensy 2.0         9        10       (none)
  Teensy++ 2.0      25         4       26, 27
  Arduino Uno        9         8         10
  Arduino Leonardo   5        13       (none)
  Arduino Mega      46        48       44, 45
  Wiring-S           5         6          4
  Sanguino          13        14         12

*/

// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable RS485 transport layer
#define MY_RS485

// Define this to enables DE-pin management on defined pin
#define MY_RS485_DE_PIN 3

// Set RS485 baud rate to use
#define MY_RS485_BAUD_RATE 115200

// Enable serial gateway
#define MY_GATEWAY_SERIAL

// Flash leds on rx/tx/err
//#define MY_LEDS_BLINKING_FEATURE
// Set blinking period
//#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Enable inclusion mode
//#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
//#define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
//#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
//#define MY_INCLUSION_MODE_BUTTON_PIN  3

//#define MY_DEFAULT_ERR_LED_PIN 4  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  5  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  6  // the PCB, on board LED

#include <SPI.h>
#include <MySensors.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>

DHT dht;
Adafruit_BMP085 bmp;
const float ALTITUDE = 250;

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_BARO 2
#define CHILD_ID_LIGHT 3

#define HUMIDITY_SENSOR_DIGITAL_PIN 5
#define LIGHT_SENSOR_ANALOG_PIN 2

// this CONVERSION_FACTOR is used to convert from Pa to kPa in forecast algorithm
// get kPa/h be dividing hPa by 10
#define CONVERSION_FACTOR (1.0/10.0)

unsigned long SLEEP_TIME = 60000; // Sleep time between reads (in milliseconds)

int minuteCount = 0;
bool firstRound = true;
// average value is used in forecast algorithm.
float pressureAvg;
// average after 2 hours is used as reference value for the next iteration.
float pressureAvg2;

float dP_dt;
boolean metric;

MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage pressureMsg(CHILD_ID_BARO, V_PRESSURE);
MyMessage forecastMsg(CHILD_ID_BARO, V_FORECAST);
MyMessage msgLight(CHILD_ID_LIGHT, V_LIGHT_LEVEL);


float lastTemp;
float lastHum;
float lastPressure;
int   lastLightLevel;
int   lastForecast;


const int LAST_SAMPLES_COUNT = 5;
float lastPressureSamples[LAST_SAMPLES_COUNT];

const char *weather[] = { "stable", "sunny", "cloudy", "unstable", "thunderstorm", "unknown" };
enum FORECAST
{
  STABLE = 0,     // "Stable Weather Pattern"
  SUNNY = 1,      // "Slowly rising Good Weather", "Clear/Sunny "
  CLOUDY = 2,     // "Slowly falling L-Pressure ", "Cloudy/Rain "
  UNSTABLE = 3,   // "Quickly rising H-Press",     "Not Stable"
  THUNDERSTORM = 4, // "Quickly falling L-Press",    "Thunderstorm"
  UNKNOWN = 5     // "Unknown (More Time needed)
};




float getLastPressureSamplesAverage()
{
  float lastPressureSamplesAverage = 0;
  for (int i = 0; i < LAST_SAMPLES_COUNT; i++)
  {
    lastPressureSamplesAverage += lastPressureSamples[i];
  }
  lastPressureSamplesAverage /= LAST_SAMPLES_COUNT;

  return lastPressureSamplesAverage;
}

// Algorithm found here
// http://www.freescale.com/files/sensors/doc/app_note/AN3914.pdf
// Pressure in hPa -->  forecast done by calculating kPa/h
int sample(float pressure)
{
  // Calculate the average of the last n minutes.
  int index = minuteCount % LAST_SAMPLES_COUNT;
  lastPressureSamples[index] = pressure;

  minuteCount++;
  if (minuteCount > 185)
  {
    minuteCount = 6;
  }

  if (minuteCount == 5)
  {
    pressureAvg = getLastPressureSamplesAverage();
  }
  else if (minuteCount == 35)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change * 2; // note this is for t = 0.5hour
    }
    else
    {
      dP_dt = change / 1.5; // divide by 1.5 as this is the difference in time from 0 value.
    }
  }
  else if (minuteCount == 65)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) //first time initial 3 hour
    {
      dP_dt = change; //note this is for t = 1 hour
    }
    else
    {
      dP_dt = change / 2; //divide by 2 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 95)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 1.5; // note this is for t = 1.5 hour
    }
    else
    {
      dP_dt = change / 2.5; // divide by 2.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 125)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    pressureAvg2 = lastPressureAvg; // store for later use.
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 2; // note this is for t = 2 hour
    }
    else
    {
      dP_dt = change / 3; // divide by 3 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 155)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 2.5; // note this is for t = 2.5 hour
    }
    else
    {
      dP_dt = change / 3.5; // divide by 3.5 as this is the difference in time from 0 value
    }
  }
  else if (minuteCount == 185)
  {
    float lastPressureAvg = getLastPressureSamplesAverage();
    float change = (lastPressureAvg - pressureAvg) * CONVERSION_FACTOR;
    if (firstRound) // first time initial 3 hour
    {
      dP_dt = change / 3; // note this is for t = 3 hour
    }
    else
    {
      dP_dt = change / 4; // divide by 4 as this is the difference in time from 0 value
    }
    pressureAvg = pressureAvg2; // Equating the pressure at 0 to the pressure at 2 hour after 3 hours have past.
    firstRound = false; // flag to let you know that this is on the past 3 hour mark. Initialized to 0 outside main loop.
  }

  int forecast = UNKNOWN;
  if (minuteCount < 35 && firstRound) //if time is less than 35 min on the first 3 hour interval.
  {
    forecast = UNKNOWN;
  }
  else if (dP_dt < (-0.25))
  {
    forecast = THUNDERSTORM;
  }
  else if (dP_dt > 0.25)
  {
    forecast = UNSTABLE;
  }
  else if ((dP_dt > (-0.25)) && (dP_dt < (-0.05)))
  {
    forecast = CLOUDY;
  }
  else if ((dP_dt > 0.05) && (dP_dt < 0.25))
  {
    forecast = SUNNY;
  }
  else if ((dP_dt > (-0.05)) && (dP_dt < 0.05))
  {
    forecast = STABLE;
  }
  else
  {
    forecast = UNKNOWN;
  }

  return forecast;
}




void setup() {
  // Setup locally attached sensors
  Serial.begin(115200);
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);
  bmp.begin();
  
}

void presentation() {
  // Present locally attached sensors
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_BARO, S_BARO);
  present(CHILD_ID_LIGHT, S_LIGHT_LEVEL);
}

void loop() {
  // Send locally attached sensor data here
  delay(dht.getMinimumSamplingPeriod());

  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    send(msgTemp.set(temperature, 1));
//    Serial.print("T: ");
//    Serial.println(temperature);
  }

  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
    Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
    lastHum = humidity;
    send(msgHum.set(humidity, 1));
//    Serial.print("H: ");
//    Serial.println(humidity);
  }
  
  float pressure = bmp.readSealevelPressure(240) / 100;
  if (pressure != lastPressure)
  {
    send(pressureMsg.set(pressure, 0));
    lastPressure = pressure;
//    Serial.print("P: ");
//    Serial.println(pressure);       
  }


  int forecast = sample(pressure);
  if (forecast != lastForecast)
  {
    send(forecastMsg.set(weather[forecast]));
    lastForecast = forecast;
//    Serial.print("forecast: ");
//    Serial.println(forecast);     
  }


  int lightLevel = (analogRead(LIGHT_SENSOR_ANALOG_PIN)) / 10.23;
  if (isnan(lightLevel) != lastLightLevel) {
    send(msgLight.set(lightLevel));
    lastLightLevel = lightLevel;
//    Serial.print("L: ");
//    Serial.println(lightLevel);    
  }



  sleep(SLEEP_TIME); //sleep a bit
}







