#include <EEPROM.h>
#include <ESP8266WiFi.h>
// when turned on: wemos will try to connect to internet via one of two wifi's.
// Wemos LED flashing rapidly indicates, connection to wifi has been established.
// LED i Wemos is lit up, when CO2 level exceeds 1000 ppm.
// In fresh air, press button connecting D8 (D3 in code) =>, ppm measure is set to 400 ppm, LED on wemos flashes 10 times. 
// Wemos LED flashes on time when it transmits to thingssepak-site. 
/*
Wemos D1 -- MH-Z14
5V -- VCC (red wire)
GND -- GND (black wire)
A0 -- Analog output (white wire)
*/

// replace with your channel’s thingspeak API key and your SSID and password
String apiKey = "";
const char* ssid = "";
const char* password = "";
const char* server = "api.thingspeak.com";
WiFiClient client;
const long limit = 1000; //ppm
long co2volt;
long co2ppm = 400;
int co2fresh = 100; //voltage-reading in fresh air, if not saved in EERPOM. Is divided by 2, as EEPROM  limit is 244.
long previousMillis = 0;
const long interval = 100000; //ca. 2 minuts interval between posting to thingsspeak
int address = 0;

void setup()
{
EEPROM.begin(512);
Serial.begin(115200);
pinMode(D3, INPUT_PULLUP); //push button betw D3 and GND.
pinMode(D4, OUTPUT); //internal LED
pinMode(D8, OUTPUT);
delay(1000);
co2fresh = EEPROM.read(address); //get the value in eeprom. Max is 244, hence multiply by 2.
delay(100);


boolean connection = false;
while (connection == false){
WiFi.begin(ssid, password);
Serial.println ("trying to connect to default");
    for (int i=0; i <= 6; i++){
    digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);              // wait for half a second
    digitalWrite(D4, LOW);  // turn the LED off by making the voltage LOW
    delay(1000);
   }

if (WiFi.status() != WL_CONNECTED) {
  WiFi.begin("SSID", "password");
  Serial.println ("trying to connect home");
    for (int i=0; i <= 3; i++){
    digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(2000);              // wait for half a second
    digitalWrite(D4, LOW);  // turn the LED off by making the voltage LOW
    delay(2000);
   }
  }

if (WiFi.status() == WL_CONNECTED) {
  connection = true;
  for (int i=0; i <= 10; i++){
    digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(50);              // wait for half a second
    digitalWrite(D4, LOW);  // turn the LED off by making the voltage LOW
    delay(50);
   }
}
}
}

void loop()
{
co2volt=analogRead(A0);
co2ppm = map(co2volt, co2fresh*2, 600, 400, 5000); //co2fresh multiply by 2, as co2fresh was /2 when it was stored
  if (co2ppm>limit) {
digitalWrite(D8, HIGH); }  // turn the LED on (HIGH is the voltage level)
else {
  digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
}

 unsigned long currentMillis = millis();
 if(currentMillis - previousMillis > interval) { //when time, run the code below
  
previousMillis = currentMillis;
co2volt=analogRead(A0);
co2ppm = map(co2volt, co2fresh*2, 600, 400, 5000); //co2fresh multiplied by 2, as co2fresh was /2 when stored.
Serial.println(co2fresh);
Serial.println(co2volt);
Serial.println(co2ppm);
 
if (client.connect(server,80) && (co2ppm>0)) { // at initialization, readings of -10000 ppm may occur
String postStr = apiKey;
postStr +="&field1=";
postStr += String(co2ppm);
postStr +="&field2=";
postStr += String(co2volt);
postStr +="&field3=";
postStr += String(co2fresh*2);
postStr += "\r\n\r\n";

client.print("POST /update HTTP/1.1\n");
client.print("Host: api.thingspeak.com\n");
client.print("Connection: close\n");
client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
client.print("Content-Type: application/x-www-form-urlencoded\n");
client.print("Content-Length: ");
client.print(postStr.length());
client.print("\n\n");
client.print(postStr);
Serial.println("sender til web");
digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
delay(500);              // wait for half a second
digitalWrite(D4, LOW);    // turn the LED off by making the voltage LOW
 }
client.stop();
// thingspeak needs at least a 15 sec delay between updates
// 20 seconds to be safe. Set 'interval' accordingly.
 }
 
 boolean btnPressed = !digitalRead(D3); //is button pressed? Press in fresh air.
    if(btnPressed == true)
    {
co2fresh=analogRead(A0)/2; //divided by 2, in order to stay within eeprom limit of 244. 
EEPROM.write(address, co2fresh); // store value in eeprom. Max 244.
delay(100);
EEPROM.commit();
 for (int i=0; i <= 10; i++){
digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
delay(100);              // wait for a second
digitalWrite(D4, LOW);    // turn the LED off by making the voltage LOW
delay(100);
    }
    }
btnPressed = false;
}

