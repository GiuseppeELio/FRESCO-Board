# PRC_Station

%%%------**HARDWARE DETAILS**------%%%

The code and the electronic sketch to build a measurement station for Passive Radiative Cooling


The system is based on a Arduino development board. In the proposed case an Arduino due has been used.

The final rendering board looks like (TOP view) \n
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Scheda_Wifi_Fan_Top.png?raw=true)


While the side view looks like
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Scheda_WiFi_Fan_3.png?raw=true)

The PCB board used as a shield board to avoid annoying wiring connection has the following electronic scheme

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/PCB_footprint_arduino%20v28.pdf?raw=true)

It is based on a 2-layer PCB. Here, the blue lines are the bottom layer and the red one is the top layer. 

%%%------**CODE**------%%%

In order to use the selected sensors such as: BH1750 (irradiance), Dallas DS18B20 (Temperature), DHT22 (temp and Humidity) and DS3231 (clocker)
it is necessary to install/ include the following libraries 

**BH1750** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/BH1750-1.3.0.zip

**Dallas DS18B20** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DS18B20-master.zip and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/Dallas_temp.zip
with the dallas libraries it is also necessary to install the **One Wire**
https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/OneWire-master.zip

**DHT22** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DHT.zip

**Clocker DS3231** and **RTC lib** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DS3231%20(1).zip
 and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/RTClib-2.0.3.zip
 and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/Arduino-Due-RTC-Library-master.zip
 
 While for the **SD card reader** (jointed with the clocker allows building a data logger) it is necessary to install: 
 https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/SD-master.zip
 
 %%%------**ARDUINO CODE**-----%%%%
 
 Now step by step the code compiled on arduino are detailed and explained. 
 
 Step 1 - Libraries
```
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <Wire.h>
//extern TwoWire Wire1;
#include <SPI.h> //for the SD card module
#include <SD.h> // for the SD card
#include "RTClib.h" // for the RTC
```

Step 2- Sensors definition 
```
#define DHTPIN 3
#define ONE_WIRE_BUS 2
#define ONE_WIRE_BUS2 4
#define ONE_WIRE_BUS3 5
#define ONE_WIRE_BUS4 6
#define DHTTYPE DHT22   // DHT 22  (AM2302)
```
Step 3- Inizialization (sensors and devices)

```
DHT dht(DHTPIN, DHTTYPE, 20); //for Arduino "due" is important to define also the clock frequency -20-
const int chipSelect = 10; //select the pin that you prefer dor the SD card reader pin
**OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);** //the bold code highlight the code used to initialize a Dallas sensors 
OneWire oneWire2(ONE_WIRE_BUS2);
DallasTemperature sensors2(&oneWire2);
OneWire oneWire3(ONE_WIRE_BUS3);
DallasTemperature sensors3(&oneWire3);
OneWire oneWire4(ONE_WIRE_BUS4);
DallasTemperature sensors4(&oneWire4);
BH1750 lightMeter; //Irradiance sensors initialized it is connected on SDA and SCL
File myFile;  // Create a file to store the data
// RTC
RTC_DS3231 rtc; // clocker initialized it is also connected on SDA and SCL
```
Step 4- Void Setup (Arduino fuction)
```
void setup() {
  Serial.begin(9600);
  dht.begin();
  sensors.begin(); //arduino starts to read data from sensors
  sensors2.begin();
  sensors3.begin();
  sensors4.begin();
  //sensors5.begin();

  Wire.begin();
  lightMeter.begin();

  //Serial.println(F("BH1750 Test begin"));
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
```
Step 5- Temperature and data reading/ storing
```
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

  //sensors5.requestTemperatures(); // Send the command to get temperatures
  //float tempC4 = sensors5.getTempCByIndex(0);

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
```

Step 6- Void Loop (Arduino function)
```
void loop() {
  loggingTemperature();
  delay(5000);
}
```
