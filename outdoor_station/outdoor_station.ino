//outdoor weather station

#include "DHT.h"   // Adafruit library https://github.com/adafruit/DHT-sensor-library  version 1.2.3
#include <Wire.h>
#include <Adafruit_BMP085.h>  // Adafruit library https://github.com/adafruit/Adafruit-BMP085-Library   version 1.0.0
Adafruit_BMP085 bmp;



#define DHTTYPE DHT22



// --- board settings ---

#define ver  "1.0.2"   // firmware version
#define addr "2"       // board address

// --- port settings ---
#define RS485controlpin   4
#define DHTPIN            5       //DHT22 
#define ledpin            13
int voltagePin =          A0;     //input voltage 
int current =             A1;     //still not used
int lightPin =            A2;     




DHT dht(DHTPIN, DHTTYPE); 

String command;


// --- functions ---
int Light() {
  return(analogRead(lightPin));
}


float Temperature() {
  delay(2000);
  for (int i = 0; i<3;i++) {
    float t = dht.readTemperature();      
    if (isnan(t)) {
      delay(2000);    
    }
    else {
      return(t);
      break;
    }
  } 
  return('err');
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

// --- setup ---
void setup() {
  Serial1.begin(9600);
  pinMode(RS485controlpin, OUTPUT);
  digitalWrite(RS485controlpin, LOW);
  dht.begin();
  bmp.begin();

}

//----------main loop------------//

void loop() {

  if(Serial1.available()) {
    String address = Serial1.readStringUntil('@');
    if (address==addr) {
    
      command = Serial1.readStringUntil('\n');

      if(command=="temp") {
        digitalWrite(RS485controlpin, HIGH);
        Serial1.println(Temperature());   
        digitalWrite(RS485controlpin, LOW);
      } 
      else if(command=="hum") {
        digitalWrite(RS485controlpin, HIGH);
        Serial1.println(Humidity());  
        digitalWrite(RS485controlpin, LOW);        
      }
      else if(command=="light") {
        digitalWrite(RS485controlpin, HIGH);
        Serial1.println(Light());
        digitalWrite(RS485controlpin, LOW);
      }
      else if (command=="pres") {
        digitalWrite(RS485controlpin, HIGH);
        Serial1.println(Pressure());
        digitalWrite(RS485controlpin, LOW);
      }
      else if (command=="volt") {
        digitalWrite(RS485controlpin, HIGH);
        Serial1.println(Voltage());
        digitalWrite(RS485controlpin, LOW);
      }
      else if (command=="ledon") {
        digitalWrite(ledpin, HIGH);
      }
      else if (command=="ledoff") {
        digitalWrite(ledpin, LOW);
      }     
    }

    command = "";    
  }


}
