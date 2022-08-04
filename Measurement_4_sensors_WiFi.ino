#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <Wire.h>
//extern TwoWire Wire1;
#include <SPI.h> //for the SD card module
#include <SD.h> // for the SD card
#include "RTClib.h" // for the RTC

#define DHTPIN 2
#define ONE_WIRE_BUS 3
#define ONE_WIRE_BUS2 4
#define ONE_WIRE_BUS3 5
#define ONE_WIRE_BUS4 6
#define esp8266 Serial3

String AP = "SSID";     // AP NAME
String PASS = "PASS WIFI"; // AP PASSWORD
String API = "API CODE";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand;
boolean found = false;

#define DHTTYPE DHT22   // DHT 22  (AM2302)


DHT dht(DHTPIN, DHTTYPE, 20);

const int chipSelect = 10;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

OneWire oneWire2(ONE_WIRE_BUS2);
DallasTemperature sensors2(&oneWire2);

OneWire oneWire3(ONE_WIRE_BUS3);
DallasTemperature sensors3(&oneWire3);

OneWire oneWire4(ONE_WIRE_BUS4);
DallasTemperature sensors4(&oneWire4);

BH1750 lightMeter;

File myFile; // Create a file to store the data

RTC_DS3231 rtc; //RTC

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
  //sendCommand("AT+CWJAP=\"" + AP + "\"", 20, "OK");
  dht.begin();

  //Serial.println("Dallas Temperature IC Control");
  sensors.begin();
  sensors2.begin();
  sensors3.begin();
  sensors4.begin();
  //sensors5.begin();

  Wire.begin();
  lightMeter.begin();
  Serial.println("DHT22, Dallas Temp IC Control and BH1750");

  // setup for the SD card
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  //open file
  myFile = SD.open("DATA.txt", FILE_WRITE);

  // if the file opened ok, write to it:
  if (myFile) {
    Serial.println("File opened ok");
    // print the headings for our data
    myFile.println("Date,Time, Umidity %, Temperature amb ºC,Temperature 1 ºC, Temperature2 ºC, Temperature3 ºC, Temperature4 ºC");
  }
  myFile.close();
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");

    // Comment out below lines once you set the date & time.
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

void loggingTemperature() {
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  //float hi = dht.computeHeatIndex(f, h);

  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC = sensors.getTempCByIndex(0);

  sensors2.requestTemperatures(); // Send the command to get temperatures
  float tempC1 = sensors2.getTempCByIndex(0);

  sensors3.requestTemperatures(); // Send the command to get temperatures
  float tempC2 = sensors3.getTempCByIndex(0);

  sensors4.requestTemperatures(); // Send the command to get temperatures
  float tempC3 = sensors4.getTempCByIndex(0);

  uint16_t lux = lightMeter.readLightLevel();
  float irr = (lux * 0.0079) * 3.9;


  DateTime now = rtc.now();
  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(now.year(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.day(), DEC);
    myFile.print('\n');
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    myFile.print(now.second(), DEC);
    myFile.print("\n");
    //
    myFile.print(h);
    myFile.println("\n");
    myFile.print(t);
    myFile.println("\n");
    myFile.print(tempC);
    myFile.println("\n");
    myFile.print(tempC1);
    myFile.println("\n");
    myFile.print(tempC2);
    myFile.println("\n");
    myFile.print(tempC3);
    myFile.println("\n");
   // myFile.print(tempC4);
   // myFile.println("\n");
    myFile.print(irr);
    myFile.println("\n");
  }
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.println(now.day(), DEC);
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  //debugging purposes
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print('\n');
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print('\n');
  //Serial.print("Heat index: ");
  //Serial.print(hi);
  //Serial.println(" *F");
  //Serial.print("\n");
  Serial.print("Temperature for the device 1 (index 0) is: ");
  Serial.println(tempC);
  Serial.print("\n");
  Serial.print("Temperature for the device 2 (index 1) is: ");
  Serial.println(tempC1);
  Serial.print("\n");
  Serial.print("Temperature for the device 3 (index 2) is: ");
  Serial.println(tempC2);
  Serial.print("\n");
  Serial.print("Temperature for the device 4 (index 3) is: ");
  Serial.println(tempC3);
  Serial.print("\n");
  //Serial.print("Temperature for the device 5 (index 4) is: ");
  //Serial.println(tempC4);
  //Serial.print("\n");
  Serial.print("Light: ");
  Serial.print(irr);
  Serial.println("W/m2");
  Serial.print(" \n");

  myFile.close();
  delay(100);

}

String getT1() {

  sensors.requestTemperatures(); // Send the command to get temperatures
  float temp = sensors.getTempCByIndex(0);
  delay(100);
  return String(temp);

}

String getT2() {

  sensors2.requestTemperatures(); // Send the command to get temperatures
  float tempC1 = sensors2.getTempCByIndex(0);
  delay(100);
  return String(tempC1);

}

String getT3() {

  sensors3.requestTemperatures(); // Send the command to get temperatures
  float tempC2 = sensors3.getTempCByIndex(0);
  delay(100);
  return String(tempC2);

}

String getT4() {

  sensors4.requestTemperatures(); // Send the command to get temperatures
  float tempC3 = sensors4.getTempCByIndex(0);
  delay(100);
  return String(tempC3);

}


String getTA() {

  float t = dht.readTemperature();
  delay(100);
  return String(t);

}

String getHA() {
  float h = dht.readHumidity();
  delay(100);
  return String(h);
}

String getIRR() {
  uint16_t lux = lightMeter.readLightLevel();
  float irr = (lux * 0.0079) * 3.9;
  delay(100);
  return String(irr);
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 0.10))
  {
    esp8266.println(command);//at+cipsend
    if (esp8266.find(readReplay)) //ok
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}

void loop() {
  loggingTemperature();
  String getData = "GET /update?api_key=" + API + "&field1=" + getTA() + "&field2=" + getHA() + "&field3=" + getT1() + "&field4=" + getT2() + "&field5=" + getT3() + "&field6=" + getT4() + "&field7=" + getIRR();
  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
  sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 4, ">");
  esp8266.println(getData); delay(1500); countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 5, "OK");
  delay(5000);
}
