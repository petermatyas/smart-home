//outdoor weather station

#include "DHT.h"   // Adafruit library https://github.com/adafruit/DHT-sensor-library  version 1.2.3
#include <Wire.h>
#include <Adafruit_BMP085.h>  // Adafruit library https://github.com/adafruit/Adafruit-BMP085-Library   version 1.0.0
Adafruit_BMP085 bmp;


//#define DHTTYPE DHT11
#define DHTTYPE DHT22


// --- board settings ---
#define ver  "1.0.5"         // firmware version
#define addr "Balcony"       // board address

// --- port settings ---
#define RS485controlpin   4
#define DHTPIN            5       //DHT22 
#define ledPin            13
int voltagePin =          A0;     //input voltage 
int currentPin =          A1;     //still not used
int lightPin =            A2;     




DHT dht(DHTPIN, DHTTYPE); 

String command;


// --- functions ---
int Light() {
  return(analogRead(lightPin));
}


float Temperature() {
  float t = dht.readTemperature();      
  return(t);
}


float Humidity() {
  delay(3000);
  for (int i = 0; i<3;i++) {
    float h = dht.readHumidity();      
    if (isnan(h)) {
      delay(2000);    
    }
    else {
      return(h);
      break;
    }
  } 
  return('err');  
}

long Pressure() {
  return(bmp.readSealevelPressure());
}

int Voltage() {
  return(analogRead(voltagePin));
}

void RS485write(String s, float p) {
  digitalWrite(RS485controlpin, HIGH);
  delay(100);
  Serial1.print("Balcony");
  Serial1.print(":");
  Serial1.print(s);
  Serial1.print(":");  
  Serial1.println(p);
  delay(100);
  digitalWrite(RS485controlpin, LOW);
}



// --- setup ---
void setup() {
  Serial1.begin(9600);
  pinMode(RS485controlpin, OUTPUT);
  digitalWrite(RS485controlpin, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  dht.begin();
  bmp.begin();

}

//----------main loop------------//

void loop() {

  if(Serial1.available()) {
    String command = Serial1.readStringUntil('\n');   


    if(command=="temp") {
      RS485write("Temp", Temperature());
    } 
    else if(command=="hum") {
      RS485write("Hum", Humidity());      
    }
    else if(command=="light") {

    }
    else if (command=="pres") {
      RS485write("Pres", Pressure());
    }
    else if (command=="volt") {

    }
    else if (command=="ledon") {
      digitalWrite(ledPin, HIGH);
    }
    else if (command=="ledoff") {
      digitalWrite(ledPin, LOW);
    }     


    command = "";    
  }


}


