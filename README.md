#       PRC Station

## Introduction and scope 

This work present a simple way to make an open source station to measure the themal heating (or cooling), humidity and solar irradicance saving the data in local and sharing them by an IOT Cloud platform. 

The final station looks like 
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/Real_PRC.jpg?raw=true)

It allows measuring the temperature from 6 sensors applied to 4 different samples (T1, T2, T3 and T4), another one inside the box (Tbox) to measure the temperature above the area covered by samples, another one is used to measure the temperature close to the electronic apparatus (identified as T_board), and another one for the ambient temperature (Tamb) and humidity. The system is also equipped with a sensor to measure the solar irradiance. Techical details about the electronic components used for this goal is reported in the next lines. All this measured value are reported in the following plot

![alt text](https://github.com/GiuseppeELio/FRESCO-Board/blob/main/Pictures/20_10_22_data.png?raw=true)
A differnet way to plot the data, the difference between the Tamb and the Tbox, and the Tsample versus Tamb and Tbox, allows to evaluate the consistence about all the sensors placed in the polystyrene box, they reports the same value in their standard deviation (accuracy ± 0.5°C).
![alt text](https://github.com/GiuseppeELio/FRESCO-Board/blob/main/Pictures/Date_28-1_10_22_Tdiff.png?raw=true)
##       Hardaware Details

The code and the electronic sketch to build a measurement station for Passive Radiative Cooling are shown in the next lines

The system is based on a Arduino development board. In the proposed case an Arduino due has been used.

The manufactured board appear as reported in the following image

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/sketch_scheda.png?raw=true)

In order to realize this Arduino shield with all connection and connector for thermal measurements, the design has been initially designed in Fusion 360, you can use a PCB software that you prefer.

First step: draw the PCB using a two layer board, top layer appears in red and the bottom one is blue. The combination of the two layers is also reported for a cohmprensive view. 

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/PCB_Wiring_sketch.png?raw=true)

Once the PCB dra is ready, the design can be inspected and produced by a virtual process, well known as CAM, thar allows seeing the final shape and the board footprint. Here, the top layer and the bottom one are reported as they will be produced during the PCB manufacturing.

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/Produced_PCB_2D.png?raw=true)

Finally, the PCB is produced by CAM, using a 3D cad software, the rendering of the final system is reported. It is useful to know the footprint of each component, the required space and how the final system will appear. 

The final rendering, along a top view and two lateral ones of the board is
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/Render_Board_annotations_2.png?raw=true)


##       Software details

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
 
 ##       ARDUINO CODE
 In the next lines you can find two version of the compiled code, the first one works only in local while the second one works as the first but includes also a wifi connection using an ESP8266 board. This implementation allows realizing an **IOT Cloud** platform exploting **ThingSpeak**. 
 
 Now step by step the code compiled on arduino are detailed and explained. 
 
 ####       Version 1
 > works only in local. 
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
#define DHTPIN 7
#define ONE_WIRE_BUS 2
#define ONE_WIRE_BUS2 3
#define ONE_WIRE_BUS3 4
#define ONE_WIRE_BUS4 5
#define ONE_WIRE_BUS5 6
#define ONE_WIRE_BUS6 8 /* Sensor on board*/
#define DHTTYPE DHT22   // DHT 22  (AM2302)
```
Step 3- Inizialization (sensors and devices)

```
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

RTC_DS3231 rtc; //clocker initialized it is also connected on SDA and SCL
```
Step 4- Void Setup (Arduino fuction)
```
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
```
Step 5- Temperature and data reading/ storing
```
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
```

Step 6- Void Loop (Arduino function)
```
void loop() {
  loggingTemperature();
  delay(5000);
}
```

####        Version 2
> It works as the one used in local but it also exploits a WIFI connection (ESP8266) to exchange data in real time with an IOT cloud platform (ThingSpeak).

> the code is https://github.com/GiuseppeELio/PRC_Station/blob/main/Measurement_4_sensors_WiFi.ino
In brief here is reported only the new part with respect to the version 1. 

Step 1 nothing change 
Step 2 - definition
```
#define esp8266 Serial1 
```
Serial1 means the Tx1 and Rx1 of arduino due, it is possible to use Serial, Serial2 or Seriel3 to use Tx and Rxor Tx2 and Rx2 or Tx3 and RX3, respectively.
Step 3 - initialization 
```
String AP = "SSID";     // AP NAME
String PASS = "PASSWORD"; // AP PASSWORD
String API = "Write API Key";   // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
```

step 4-  Void setup **new part have been included**
```
esp8266.begin(115200);
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
```
Step 5 - nothing change
Step 5.1- It is a new step that include the conversion of the collected data into string
```
String getT1() {

  sensors.requestTemperatures(); // Send the command to get temperatures
  float tempC1 = sensors.getTempCByIndex(0);
  delay(100);
  return String(tempC1);

}

String getT2() {

  sensors2.requestTemperatures(); // Send the command to get temperatures
  float tempC2 = sensors2.getTempCByIndex(0);
  delay(100);
  return String(tempC2);

}

String getT3() {

  sensors3.requestTemperatures(); // Send the command to get temperatures
  float tempC3 = sensors3.getTempCByIndex(0);
  delay(100);
  return String(tempC3);

}

String getT4() {

  sensors4.requestTemperatures(); // Send the command to get temperatures
  float tempC4 = sensors4.getTempCByIndex(0);
  delay(100);
  return String(tempC4);

}

String getT5() {

  sensors5.requestTemperatures(); // Send the command to get temperatures
  float tempC5 = sensors5.getTempCByIndex(0);
  delay(100);
  return String(tempC5);

}

String getT_oB() {

  sensors6.requestTemperatures(); // Send the command to get temperatures
  float tempC_oB = sensors6.getTempCByIndex(0);
  delay(100);
  return String(tempC_oB);

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
```
and Finally 
Step 6- Void Loop 
```
void loop() {
  loggingTemperature();
String getData = "GET /update?api_key=" + API + "&field1=" + getTA() + "&field2=" + getHA() + "&field3=" + getT1() + "&field4=" + getT2() + "&field5=" + getT3() + "&field6=" + getT4() + "&field7=" + getT5() + "&field8=" + getIRR();
  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
  sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 4, ">");
  esp8266.println(getData); delay(1500); countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 5, "OK");
  delay(5000);
}
```
getT1, getT2, getT3, getT4 and getT5 are related to the sensors placed above the samples or into the box to monitor the "ambient/box" temperature. getIRR is used for the irradiance and getTA and getHA for the ambient temperature and humidity, respectively. It exist also the function getT_oB that is not recalled in line #441 and it represents the temperature read on the board, close to the electronic components. 
Here, in the last step is important to point out our attention on the following code line
```
String getData = "GET /update?api_key=" + API + "&field1=" + getTA() + "&field2=" + getHA() + "&field3=" + getT1() + "&field4=" + getT2() + "&field5=" + getT3() + "&field6=" + getT4() + "&field7=" + getT5() + "&field8=" + getIRR();
```
that is used to exchange the data converted into string with the ThingSpeak channel using AT command. 
