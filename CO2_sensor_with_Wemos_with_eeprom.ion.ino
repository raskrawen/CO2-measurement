#include <EEPROM.h>
#include <ESP8266WiFi.h>
// LED i Wemos pin D10 (D8 i koden) lyser permanent, når CO2 bliver over 1000 ppm.
// I frisk luft: Knap i D8 (D3 i koden) presses, ppm sættes til 400, blinker LED i D10 10 gange 
// indbygget LED (D4 i koden) blinker, når der sendes til net. 
/*
Wemos D1 -- MH-Z14
5V -- VCC
GND -- GND
A0 -- Analog output
*/

// replace with your channel’s thingspeak API key and your SSID and password
String apiKey = "";
const char* ssid = "SGHF-guest-net";
const char* password = "";
const char* server = "api.thingspeak.com";
WiFiClient client;
const long limit = 2000; //ppm
long co2volt;
long co2ppm = 400;
int co2fresh = 105; //volt i frisk luft, hvis der ikke er gemt andet i eeprom. Er x½ da EEPROM max kan holde 244.
long previousMillis = 0;
const long interval = 1000000; //ca. 20 minutter
int address = 0;

void setup()
{
EEPROM.begin(512);
Serial.begin(115200);
pinMode(D3, INPUT_PULLUP); //push button betw D3 and GND.
pinMode(D4, OUTPUT); //internal LED
pinMode(D8, OUTPUT);
delay(1000);
co2fresh = EEPROM.read(address); //hente værdi i eeprom. Må max være 244, skal derfor ganges med 10
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
  WiFi.begin("camilla-PC-Wireless", "");
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
 unsigned long currentMillis = millis();
 if(currentMillis - previousMillis > interval) { //when time, run the code below
  
previousMillis = currentMillis;
co2volt=analogRead(A0);
co2ppm = map(co2volt, co2fresh*2, 600, 400, 5000); //co2fresh ganges med to, da co2fresh blev /2 da den blev gemt
Serial.println(co2fresh);
Serial.println(co2volt);
Serial.println(co2ppm);
if (co2ppm>limit) {
digitalWrite(D8, HIGH); }  // turn the LED on (HIGH is the voltage level)
else {
  digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
}

 
if (client.connect(server,80) && (co2ppm>0)) { // ved opstart kan forekomme værdier på -10000 ppm
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
 
 boolean btnPressed = !digitalRead(D3); //is button pressed?
    if(btnPressed == true)
    {
co2fresh=analogRead(A0)/2; //divideres med to for at den kan være i EEPROM. 
EEPROM.write(address, co2fresh); //gem værdi i eeprom. Må max være 244.
delay(100);
EEPROM.commit();
 for (int i=0; i <= 10; i++){
digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
delay(100);              // wait for a second
digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
delay(100);
    }
    }
btnPressed = false;
}

