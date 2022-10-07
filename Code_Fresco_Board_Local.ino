#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <Wire.h>
//extern TwoWire Wire1;
#include <SPI.h> //for the SD card module
#include <SD.h> // for the SD card
#include "RTClib.h" // for the RTC

#define DHTPIN 7
#define ONE_WIRE_BUS 2
#define ONE_WIRE_BUS2 3
#define ONE_WIRE_BUS3 4
#define ONE_WIRE_BUS4 5
#define ONE_WIRE_BUS5 6
#define ONE_WIRE_BUS6 8 /* Sensor on board*/

#define DHTTYPE DHT22   // DHT 22  (AM2302)


DHT dht(DHTPIN, DHTTYPE, 20);

const int chipSelect = 9;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

OneWire oneWire2(ONE_WIRE_BUS2);
DallasTemperature sensors2(&oneWire2);

OneWire oneWire3(ONE_WIRE_BUS3);
DallasTemperature sensors3(&oneWire3);

OneWire oneWire4(ONE_WIRE_BUS4);
DallasTemperature sensors4(&oneWire4);

OneWire oneWire5(ONE_WIRE_BUS5);
DallasTemperature sensors5(&oneWire5);

OneWire oneWire6(ONE_WIRE_BUS6);
DallasTemperature sensors6(&oneWire6);

BH1750 lightMeter;

File myFile; // Create a file to store the data

RTC_DS3231 rtc; //RTC

void setup() {
  Serial.begin(9600);
  dht.begin();
  //Serial.println("Dallas Temperature IC Control");
  sensors.begin();
  sensors2.begin();
  sensors3.begin();
  sensors4.begin();
  sensors5.begin();
  sensors6.begin();
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
    myFile.println("Date,Time, Umidity %, Temperature amb ºC,Temperature 1 ºC, Temperature2 ºC, Temperature3 ºC, Temperature4 ºC,Temperature5 ºC,Temperature_On_Board ºC");
  }
  myFile.close();
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    //Comment out below lines once you set the date & time.
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void loggingTemperature() {
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC1 = sensors.getTempCByIndex(0);

  sensors2.requestTemperatures(); // Send the command to get temperatures
  float tempC2 = sensors2.getTempCByIndex(0);

  sensors3.requestTemperatures(); // Send the command to get temperatures
  float tempC3 = sensors3.getTempCByIndex(0);

  sensors4.requestTemperatures(); // Send the command to get temperatures
  float tempC4 = sensors4.getTempCByIndex(0);

  sensors5.requestTemperatures(); // Send the command to get temperatures
  float tempC5 = sensors5.getTempCByIndex(0);

  sensors6.requestTemperatures(); // Send the command to get temperatures
  float tempC_oB = sensors6.getTempCByIndex(0);
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
    myFile.print(tempC1);
    myFile.println("\n");
    myFile.print(tempC2);
    myFile.println("\n");
    myFile.print(tempC3);
    myFile.println("\n");
    myFile.print(tempC4);
    myFile.println("\n");
    myFile.print(tempC5);
    myFile.println("\n");
    myFile.print(tempC_oB);
    myFile.println("\n");
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
  Serial.println(tempC1);
  Serial.print("\n");
  Serial.print("Temperature for the device 2 (index 1) is: ");
  Serial.println(tempC2);
  Serial.print("\n");
  Serial.print("Temperature for the device 3 (index 2) is: ");
  Serial.println(tempC3);
  Serial.print("\n");
  Serial.print("Temperature for the device 4 (index 3) is: ");
  Serial.println(tempC4);
  Serial.print("\n");
  Serial.print("Temperature for the device 5 (index 4) is: ");
  Serial.println(tempC5);
  Serial.print("\n");
    Serial.print("Temperature on Board (index 5) is: ");
  Serial.println(tempC_oB);
  Serial.print("\n");
  Serial.print("Light: ");
  Serial.print(irr);
  Serial.println("W/m2");
  Serial.print(" \n");

  myFile.close();
  delay(100);

}

void loop() {
  loggingTemperature();
  delay(5000);
}
