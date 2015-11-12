#include "DHT.h"   // Adafruit library https://github.com/adafruit/DHT-sensor-library  version 1.2.3
#include <Wire.h>

//#define DHTTYPE DHT11
#define DHTTYPE DHT22

// --- board settings ---
#define ver  "1.0.3"           // firmware version
#define addr "SmallRoom:"      // board address


// --- port settings ---
#define RS485controlpin   4
#define DHTPIN            5
#define wateringPin       6      
#define ledPin            13
int voltagePin =          A0;
int currentPin =          A1;
int lightPin =            A2;


DHT dht(DHTPIN, DHTTYPE); 



// --- settings ---

int  wateringDelay = 2000;
float t,h;
unsigned long previousMillis,currentMillis;

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
  if (currentMillis - previousMillis > 2000) {
    for (int i = 0; i < 3; i++) {
      t = dht.readTemperature();
      if (isnan(t)) {
        delay(2000);
        }
        else {
          break;
        }
      }  
    for (int i = 0; i < 3; i++) {
      h = dht.readHumidity();
      if (isnan(h)) {
        delay(2000);
        }
        else {
          break;
        }
      }  
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


void setup() {
  Serial.begin(9600);
  pinMode(RS485controlpin, OUTPUT);
  digitalWrite(RS485controlpin, LOW);
  pinMode(wateringPin, OUTPUT);      
  digitalWrite(wateringPin, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  dht.begin();
  
  previousMillis = millis();
}

//----------main loop------------//

void loop() {
  unsigned long currentMillis = millis();
  String command,segedstr;
  static char outstr[15];

  if(Serial.available()){
    command = Serial.readStringUntil('\n');

    if(command.substring(0,sizeof("watering")-1)=="watering"){
      segedstr = command.substring(sizeof("watering"));
      watering(segedstr.toInt());
    }

    if(command=="getSmallRoomTemp") {
      dtostrf(Temperature(),3,1,outstr);
      segedstr = "SmallRoom:Temp:";
      segedstr += outstr;
      Serial.println(segedstr);
    } 
    else if(command=="getSmallRoomHum") {
      dtostrf(Humidity(),3,1,outstr);
      segedstr = "SmallRoom:Temp:";
      segedstr += outstr;
      Serial.println(segedstr);         
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
      Serial.println("ok");     
    }     
    
    command = "";    
  }


}
