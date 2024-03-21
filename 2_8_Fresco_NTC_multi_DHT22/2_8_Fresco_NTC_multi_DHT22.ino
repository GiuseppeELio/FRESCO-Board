/* Libraries */
#include <arduino-timer.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BH1750.h>
#include <Wire.h>
#include <SPI.h>     //for the SD card module
#include <SD.h>      // for the SD card
#include "RTClib.h"  // for the RTC
#include <Adafruit_MLX90614.h>
#include "U8glib.h"
/*Timer definition*/
Timer<4> timer;                                                   // create a timer with N tasks and microsecond resolution
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);  // I2C / TWI
/*DEFINITION*/
#define DHTPIN 13
#define DHTPIN2 14
#define DHTPIN3 15
#define ONE_WIRE_BUS2 8 /* Sensor on board*/ /*Remember to not put it in the parallel line of the sensors that go in the external box*/
#define DHTTYPE DHT22                        // DHT 22  (AM2302)
#define esp8266 Serial2                      //RX and TX settled on Serial2 --> RX2 and TX2 of Arduino Mega
/*NTC parameters*/
#define RT0 10000  // Ω
#define B 3380     //  K NTC part number NXFT15XH103FA2B100 or 3455
#define VCC 5      //Operating Voltage
#define R 10000    //R=10KΩ Pull Up resistor
#define T0 298.15
/*Task times */
//#define RESET_TIME 14400000 //time for reset function every 4 hours in milliseconds -- 7200000 (2 hours)
//unsigned long ResetTime = RESET_TIME;
#define TASK1 1000                  //gettemp
unsigned long TASK2 = 5500;  //Log data and send them using string
unsigned long TASK3 = 3000;
/* Values Initialiazation */
float tempBoard = 0;  //The sensor used to measure the temp. on Board
float t, h, t2, h2, t3, h3, irr, IR_temp_amb, IR_temp_sky = 0;
// NTC variables initialization
const int numChannels = 5;
const int numOtherSensors = 10;
int analogPins[numChannels] = {A0, A1, A2, A3, A4};
float VRT[numChannels];
float VR[numChannels];
float RT[numChannels];
float ln[numChannels];
float T[numChannels];
/**/
String str0;
String str1;
String str;
String clock_status;
String Date_Time;
String sd_status;
String wifi_status;
String Date;
String Time;
String apIP;
String localIP;
/* Temperature Sensors  */
DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);
/* On board using DS18B20 */
OneWire oneWireBoard(ONE_WIRE_BUS2);
DallasTemperature sensorsBoard(&oneWireBoard);
/* LIGHT Sensor */
BH1750 lightMeter;
/* IR Sensor for Ambient and Sky temperature */
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
/* SD CARD */
const int chipSelect = 10;  //Pin used for SD module (Do not change it! it is defined on the PCB board)
File myFile;                // Create a file to store the data
/* Real Time Clock */
RTC_DS3231 rtc;  //RTC

void setup() {
  Serial.begin(115200);
  esp8266.begin(115200);
  /**/
  //T0 = 25 + 273.15;
  /**/
  Sensors_initialization();
  RTC_initialization();
  SD_initialization();
  delay(30000); //to be setted again at 15000
  Wifi_status();
  init_screen();
  /**/
  timer.every(TASK1, datalog);
  timer.every(TASK2,getstring);
  timer.every(TASK3, displaying_data);
  //timer.every(ResetTime, resetFunc);
  auto active_tasks = timer.size();
  Serial.print("Active task:");
  Serial.println(active_tasks);
}

void loop() {
  timer.tick();
}
