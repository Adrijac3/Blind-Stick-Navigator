

#include "TinyGPS++.h"
#include "SoftwareSerial.h"

#define sos 7 // SOS BUTTON
#define smsLed 9
#define smsLedDelay 1000

int tonePin = 4;    //Tone - Red Jumper
int trigPin = 5;    //Trig - violet Jumper
int echoPin = 6;   //Echo - yellow Jumper

int proximity=0;
int duration;
int distance;

SoftwareSerial gsm(10,11);//RX=pin 10 Tx=pin 11
TinyGPSPlus gps;//RX=pin 0 Tx=pin 1

String message = "Lat: ,\nLong: ";
String lat = "", lng = "";

String phone = "9xxxxxxxxx";
unsigned long previousMillis = 0;
int buttonState = 0;  // 0 = not pressed   --- 1 = long pressed --- 2 short pressed
int DURATION_IN_MILLIS = 2000; // FOR BUTTON PRESS & HOLD
 
void setup() {
  //Serial Port
  Serial.begin (9600);
  gsm.begin(9600);

  pinMode(13, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(tonePin, OUTPUT);
  pinMode(smsLed, OUTPUT);
  pinMode(sos,INPUT);

  gsm.print("AT");
  delay(2000);
  if(gsm.readString().indexOf("OK")== -1) {
    digitalWrite(13,HIGH);// idicating GSM fail
    delay(2000);
  }
  digitalWrite(13,LOW);
  gsm.println("AT+CMGF=1");
  delay(2000);

  // GPS Setup
  delay(2000);
  Serial.println("GPS Start");
}
 
void loop() {
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  
  proximity=map(distance, 0, 100, 8, 0);
  Serial.println(proximity);

  if (proximity <= 0){
    proximity=0;
  }
  else if (proximity >= 3 && proximity <= 4){
    tone(tonePin, 20000, 200);
  }
  else if (proximity >= 5 && proximity <= 6){
     tone(tonePin,3000, 200);
  }
  else if (proximity >= 7 && proximity <= 8){
    tone(tonePin, 1000, 200);
  }
  
  delay(400);
  noTone(tonePin);

  readGPS();
  checkSOSSwitchStatus();
}

void readGPS() {
  
  while(Serial.available()) {
      gps.encode(Serial.read());
  }

  lat = String(gps.location.lat(),4);
  lng = String(gps.location.lng(),4);

  Serial.println(lat + " " + lng);
}

void checkSOSSwitchStatus() {
  
  buttonState = 0;
  if(digitalRead(sos)){
    previousMillis = millis();
    buttonState = 1;
    while((millis()-previousMillis)<=DURATION_IN_MILLIS){
      if(!(digitalRead(sos))){
        buttonState = 2;
        break;
      }
    }
  }
  
  if(!buttonState){
    // TODO nothing is pressed
  }else if(buttonState == 1){
    SMSsender(phone,"Emergency Service Required");
  }else if(buttonState ==2){
    //TODO button is pressed short
  }
}

void SMSsender(String n, String txt) {
  
    digitalWrite(smsLed, HIGH);
    String message = "\nLat: " + lat + ",\nLong: " + lng;
    message = txt + message + "\n\nhttps://www.google.com/maps/search/?api=1&query=" + lat + "," + lng;
    gsm.println("AT+CMGS=\""+n+"\"");
    while(gsm.read()!='>');
    gsm.print(message);
    gsm.write(0x1A);
    gsm.write(0x0D);
    gsm.write(0x0A);
    delay(smsLedDelay);
    digitalWrite(smsLed, LOW);
}
