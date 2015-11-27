//outdoor weather station

#include "DHT.h"   // Adafruit library https://github.com/adafruit/DHT-sensor-library  version 1.2.3
#include <Wire.h>
#include <Adafruit_BMP085.h>  // Adafruit library https://github.com/adafruit/Adafruit-BMP085-Library   version 1.0.0
Adafruit_BMP085 bmp;


#define NodeId 1
#define ChildSensorId 5

//message type
#define presentation 0
#define set 1
#define req 2
#define internal 3

//presentation
#define S_TEMP 6
#define S_HUM 7
#define S_BARO 8

//set req
#define V_TEMP 0
#define V_HUM 1
#define V_PRESSURE 4




//#define DHTTYPE DHT11
#define DHTTYPE DHT22


// --- board settings ---
#define ver  "1.0.6"         // firmware version
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

void RS485write(int MessageType, int Ack, int SubType, float PayLoad) {
  digitalWrite(RS485controlpin, HIGH);
  delay(100);
  Serial1.print(NodeId);
  Serial1.print(";");
  Serial1.print(ChildSensorId);
  Serial1.print(";");  
  Serial1.print(MessageType);
  Serial1.print(";");
  Serial1.print(Ack);
  Serial1.print(";");  
  Serial1.print(SubType);
  Serial1.print(";");  
  Serial1.println(PayLoad);    
  delay(100);
  digitalWrite(RS485controlpin, LOW);
}

void SendPresentation() {
  RS485write(presentation,0,S_TEMP,0);
  RS485write(presentation,0,S_HUM,0);     




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
      RS485write(set,0,S_TEMP,Temperature());
    } 
    else if(command=="hum") {
      //RS485write("Hum", Humidity());      
    }
    else if(command=="light") {

    }
    else if (command=="pres") {
      //RS485write("Pres", Pressure());
    }
    else if (command=="volt") {
      //RS485write("Volt", Voltage());
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


