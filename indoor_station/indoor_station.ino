//indoor station

#include "DHT.h"   // Adafruit library https://github.com/adafruit/DHT-sensor-library  version 1.2.3
#include <Wire.h>

//#define DHTTYPE DHT11
#define DHTTYPE DHT22

// --- board settings ---
#define ver  "1.0.5"           // firmware version
#define addr "SmallRoom"       // board address


// --- port settings ---
#define RS485controlpin   4
#define DHTPIN            5
#define wateringPin       6      
#define ledPin            13
int voltagePin =          A0;
int currentPin =          A1;
int lightPin =            A2;


DHT dht(DHTPIN, DHTTYPE); 

;


// --- settings ---

int  wateringDelay = 2000;
float t,h;
unsigned long currentMillis = millis(),previousMillis;
String command,segedstr;
static char outstr[15];



void watering(int t) {
  Serial.println("in wateing");
  digitalWrite(wateringPin, HIGH);
  delay(t);
  digitalWrite(wateringPin, LOW);

}

int Light() {
  return(analogRead(lightPin));
}

float dhtth() {
  currentMillis = millis();
  if (currentMillis - previousMillis > 3000) {
      t = dht.readTemperature();
      h = dht.readHumidity();
      previousMillis = currentMillis;
    }
  
  }

float Temperature() {
  dhtth();
  return(t);
}

float Humidity() {
  dhtth();
  return(h);
}

void RS485write(String s1, String s2) {
  digitalWrite(RS485controlpin, HIGH);
  delay(100);
  Serial1.print(addr);
  Serial1.print(":");
  Serial1.print(s1);
  Serial1.print(":");  
  Serial1.println(s2);
  digitalWrite(RS485controlpin, LOW);
}



void setup() {
  Serial1.begin(9600);
  pinMode(RS485controlpin, OUTPUT);
  digitalWrite(RS485controlpin, LOW);
  pinMode(wateringPin, OUTPUT);      
  digitalWrite(wateringPin, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  dht.begin();
  
  previousMillis = millis();
  t = dht.readTemperature();
  h = dht.readHumidity();
}

//----------main loop------------//

void loop() {

  if(Serial1.available()){
    command = Serial1.readStringUntil('\n');

    if(command.substring(0,sizeof("watering")-1)=="watering"){
      segedstr = command.substring(sizeof("watering"));
      watering(segedstr.toInt());
    }

    if(command=="getSmallRoomTemp") {
      dtostrf(Temperature(),3,1,outstr);
      RS485write("Temp", outstr);
    } 
    else if(command=="getSmallRoomHum") {
      //dtostrf(Humidity(),3,1,outstr);
      //segedstr = addr;
      //segedstr += ":Hum:";
      //segedstr += outstr;
      //RS485write(segedstr);       
      dtostrf(Humidity(),3,1,outstr);
      RS485write("Hum", outstr)     
    }
    else if(command=="light") {
      Serial.print(addr);
      Serial.print("Light:");
      Serial.println(Light());
    }
    else if(command=="waterlv") {
      Serial.print(addr);
      Serial.print("Waterlv:");
      Serial.println("0");
    }
    else if(command=="ledon") {
      digitalWrite(ledPin, HIGH);
    } 
    else if(command=="ledoff") {
      digitalWrite(ledPin, LOW);     
    }     
    else if(command=="statue") {
      RS485write("ok");     
    }     
      
    
    
    command = "";    
  }


}
