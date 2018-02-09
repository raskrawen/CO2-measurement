#include <EEPROM.h>
#include <ESP8266WiFi.h>
// LED i Wemos pin D8 til D13 (D3 til D8 i koden) lyser , når CO2 går over hhv. 500, 750, 1000, 1250, 1500, 1750 ppm.
// I frisk luft: Forbind D14 (D2 i koden) med GND, ppm sættes til 400, blinker indbygget LED (D4 i koden) 10 gange
// indbygget LED (D4 i koden) blinker, når der sendes til net.
/*
  Wemos D1 -- MH-Z14
  5V -- VCC
  GND -- GND
  A0 -- Analog output
*/

/*
  API keys:
  1 L9JW4G9QFJC3QPTD
  2 FN8SWACJSOY6PGO7
  3 BCGXB8RHPV71VHQD
  4 3X23XXMCGT3V0QJ1

*/


// replace with your channel’s thingspeak API key and your SSID and password
String apiKey = "BCGXB8RHPV71VHQD";
const char* ssid = "SGHF-guest-net";
const char* password = " PASSWORD";
const char* server = "api.thingspeak.com";
WiFiClient client;
const long limit = 1000; //ppm
long co2volt;
long co2ppm = 400;
int co2fresh = 105; //volt i frisk luft, hvis der ikke er gemt andet i eeprom. Er x½ da EEPROM max kan holde 244.
long previousMillis = 0;
const long interval = 100000; //10000 = 
int address = 0;
int k; //loop that accumulates reference values.
long sumOfData = 0;

void setup()
{
  EEPROM.begin(512);
  Serial.begin(115200);
  Serial.println ("START");
  pinMode(D2, INPUT_PULLUP); //push button betw D7 (D2 in code) and GND.
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT); //internal LED
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  co2fresh = EEPROM.read(address); //hente værdi i eeprom. Må max være 244, skal derfor ganges med 2
  delay(100);


  boolean connection = false;
  while (connection == false) {
    Serial.println ("trying to connect to default");
    WiFi.begin(ssid, password);
    for (int i = 0; i <= 6; i++) {
      digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);              // wait for half a second
      digitalWrite(D4, LOW);  // turn the LED off by making the voltage LOW
      delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println ("trying to connect home");
      WiFi.begin("camilla-PC-Wireless", " PASSWORD");
      for (int i = 0; i <= 3; i++) {
        digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(2000);              // wait for half a second
        digitalWrite(D4, LOW);  // turn the LED off by making the voltage LOW
        delay(2000);
      }
    }

    if (WiFi.status() == WL_CONNECTED) {
      connection = true;
      for (int i = 0; i <= 10; i++) {
        digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(50);              // wait for half a second
        digitalWrite(D4, LOW);  // turn the LED off by making the voltage LOW
        delay(50);
      }
    }
  }
  Serial.println ("Connected.");
  Serial.println("2 minutes warm up..");
  delay(120000); // give time for the sensor to initialize.
}

void loop()
{
  co2volt = analogRead(A0);
  co2ppm = map(co2volt, co2fresh * 2, 600, 400, 5000); //co2fresh ganges med to, da co2fresh blev /2 da den blev gemt
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
  if (co2ppm > 500) {
    digitalWrite(D8, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  if (co2ppm > 750) {
    digitalWrite(D7, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  if (co2ppm > 1000) {
    digitalWrite(D6, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  if (co2ppm > 1250) {
    digitalWrite(D5, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  if (co2ppm > 1500) {
    digitalWrite(D4, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  if (co2ppm > 1725) {
    digitalWrite(D3, HIGH); // turn the LED on (HIGH is the voltage level)
  }
  delay(100);


unsigned long currentMillis = millis();
if (currentMillis - previousMillis > interval) { //when time, run the code below

  previousMillis = currentMillis;
  co2volt = analogRead(A0);
  co2ppm = map(co2volt, co2fresh * 2, 600, 400, 5000); //co2fresh ganges med to, da co2fresh blev /2 da den blev gemt
  Serial.print("CO2freshx2: ");
  Serial.println(co2fresh * 2);
  Serial.print("CO2volt: ");
  Serial.println(co2volt);
  Serial.print("CO2ppm: ");
  Serial.println(co2ppm);

  if (k < 50) { //loop for collecting 10 reference points. If less than previous calibration => new calibration
    sumOfData += co2volt;
    Serial.print("k: ");
    Serial.println(k);
    Serial.print("SumOfData: ");
    Serial.println(sumOfData);
    k = k + 1;
  }
  else {  //when k=50
    Serial.println("Checking ref.");
    Serial.print("Old refx50: ");
    Serial.println(co2fresh * 100); //co2fresh is already x½ cause of EEPROM.
    Serial.print("New Datax50: ");
    Serial.println(sumOfData);
    if (co2fresh * 100 > sumOfData) { //if new data is less than reference-value. co2freshx2 cause co2fresh is ½ of value.
      Serial.print("Storing new ref point.");
      co2fresh = co2volt / 2;
      EEPROM.write(address, co2fresh); //gem værdi i eeprom. Må max være 244.
      delay(100);
      EEPROM.commit();
    }
    k = 0; //reset k
    sumOfData = 0; //reset sum
  }

  if (client.connect(server, 80) && (co2ppm > 0)) { // ved opstart kan forekomme værdier på -10000 ppm
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(co2ppm);
    postStr += "&field2=";
    postStr += String(co2volt);
    postStr += "&field3=";
    postStr += String(co2fresh * 2);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
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

boolean btnPressed = !digitalRead(D2); //is button pressed?
if (btnPressed == true)
{
  co2fresh = analogRead(A0) / 2; //divideres med to for at den kan være i EEPROM.
  EEPROM.write(address, co2fresh); //gem værdi i eeprom. Må max være 244.
  delay(100);
  EEPROM.commit();
  for (int i = 0; i <= 10; i++) {
    digitalWrite(D4, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);              // wait for a second
    digitalWrite(D4, LOW);    // turn the LED off by making the voltage LOW
    delay(100);
  }
  k = 0; //reset k
  sumOfData = 0; //reset sum

}
btnPressed = false;
}
