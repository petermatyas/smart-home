#version 1.0.1

#include "DHT.h"

DHT dht;


// --- port settings ---
#define wateringPin   4      // the number of the LED pin
#define DHTpin 5


int t,intseged;       //intseged: egesz tipusu seged valtozo;   t:öntözés ideje
String command,segedstr;   //command: soros porton beérkező parancs; 

int soilHumidityPin3cm = A0;
int soilHumidityPin6cm = A1;
int lightPin = A2;

long previousMillis0 = 0;  
long previousMillis1 = 0;  
long duration, distance;


// --- settings ---
long wateringInterval = 8866800000;    // 1000*60*60*24*3   3 days
long measuringInterval = 3000;
int  wateringDelay = 2000;

void setup() {
  Serial.begin(9600);
  pinMode(wateringPin, OUTPUT);      
  digitalWrite(wateringPin, LOW);

  dht.setup(DHTpin);  

}

void watering(int t) {
  Serial.println("in wateing");
  digitalWrite(wateringPin, HIGH);
  delay(t);
  digitalWrite(wateringPin, LOW);

}

int soilHumidity3cm() {
  return(analogRead(soilHumidityPin3cm)); 
}

int soilHumidity6cm() {
  return(analogRead(soilHumidityPin6cm)); 
}

int Light() {
  return(analogRead(lightPin));
}


float Temperature() {
  delay(dht.getMinimumSamplingPeriod());
  //delay(2000);
  return(dht.getTemperature());
}

float Humidity() {
  delay(dht.getMinimumSamplingPeriod());
  //delay(3000);
  return(dht.getHumidity());
}

//----------main loop------------//

void loop() {
  unsigned long currentMillis = millis();


  //  if(currentMillis - previousMillis0 > wateringInterval) {
  //    previousMillis0 = currentMillis;   
  //    watering(wateringDelay);
  //  }

  //  if(currentMillis - previousMillis1 > measuringInterval) {
  //    previousMillis1 = currentMillis;   
  //Serial.println(soilHumidity3cm());
  //Serial.println(soilHumidity6cm());  
  //  }



  if(Serial.available()){
    command = Serial.readStringUntil('\n');

    if(command.substring(0,sizeof("watering")-1)=="watering"){
      segedstr = command.substring(sizeof("watering"));
      watering(segedstr.toInt());
    }

    if(command=="hum3cm") {
      Serial.println(soilHumidity3cm());  
    } 
    else if(command=="hum6cm") {
      Serial.println(soilHumidity6cm());          
    } 
    else if(command=="temp") {
      Serial.println(Temperature());     
    } 
    else if(command=="hum") {
      Serial.println(Humidity());          
    }
    else if(command=="light") {
      Serial.println(Light());
    }
    else if(command=="waterlv") {
      Serial.println("0");
    }

    command = "";    
  }




  /*  Serial.print("lev: ");
   Serial.println(waterLevel());
   Serial.print("hum3cm: ");
   Serial.println(soilHumidity3cm());
   Serial.print("hum6cm: ");
   Serial.println(soilHumidity6cm());
   delay(1000);
   */

}