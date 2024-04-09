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
#define esp8266 Serial1                      //RX and TX settled on Serial2 --> RX2 and TX2 of Arduino Mega
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


void RTC_initialization() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  //rtc.adjust(DateTime((__DATE__), (__TIME__))); // Use this command in order to set the rigth Date and time on the RTC
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    //Comment out below lines once you set the date & time.
    rtc.adjust(DateTime((__DATE__), (__TIME__)));
  }
}

// setup for the SD card
void SD_initialization() {
  if (!SD.begin(chipSelect)) {
    Serial.println("SD initialization failed!");
    sd_status = "Fail";
    return;
  }
  Serial.println(F("initialization done."));
  sd_status = "Done";

  // Get today's filename
  String filename = getTodayFilename();

  // Create or open the file
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    Serial.println("File created: " + filename);
    dataFile.println("New data acquisition");
    dataFile.println("Date & Time, TA1, H1, TA2, H2, TA3, H3, TS1, TS2, TS3, TS4, Tbx, Tbr, Ir, TAIR, TSIR");
    Serial.println("Date & Time, Tamb 1, H 1, Tamb2, H2, Tamb 3, H3, TS1, TS2, TS3, TS4, Tbox, Tboard, Irr, Tamb IR, TSky IR");
    dataFile.close();
  } else {
    Serial.println("Error creating file: " + filename);
  }
}

String getTodayFilename() {
  // Get current date
  DateTime now = rtc.now();  // Assuming you have an RTC named "rtc" initialized

  // Create folder structure for year, month, and day
  String folderPath = "/" + String(now.year()) + "/" + formatTwoDigits(now.month()) + "/" + formatTwoDigits(now.day()) + "/";
  SD.mkdir(folderPath);  // Create folders if they don't exist

  // Create filename for today's data
  String filename = folderPath + "data.txt";

  return filename;
}

String formatTwoDigits(int number) {
  if (number < 10) {
    return "0" + String(number);
  } else {
    return String(number);
  }
}

void Wifi_status() {
  while (esp8266.available() > 0) {
    char received = esp8266.read();
    Serial.print("Received: ");
    Serial.println(received);
    if (received == 'C') {
      wifi_status = "Done";
      Serial.println("wifi ok");
      // Read and update access point IP
      apIP = "";
      while (esp8266.available() > 0) {
        char apIPChar = esp8266.read();
        if (apIPChar == '\n') {
          break;
        }
        apIP += apIPChar;
      }
      Serial.print("Access Point IP: ");
      Serial.println(apIP);
      // Read and update local WiFi IP
      localIP = "";
      while (esp8266.available() > 0) {
        char localIPChar = esp8266.read();
        if (localIPChar == '\n') {
          break;
        }
        localIP += localIPChar;
      }
      Serial.print("Local WiFi IP: ");
      Serial.println(localIP);

      getDateTimeFromESP();
    } else if (received == 'D') {
      wifi_status = "Fail";
      Serial.println("wifi fail");
      // Read and update access point IP
      apIP = "";
      while (esp8266.available() > 0) {
        char apIPChar = esp8266.read();
        if (apIPChar == '\n') {
          break;
        }
        apIP += apIPChar;
      }
      Serial.print("Access Point IP: ");
      Serial.println(apIP);
      // Read and update local WiFi IP
      localIP = "";
      while (esp8266.available() > 0) {
        char localIPChar = esp8266.read();
        if (localIPChar == '\n') {
          break;
        }
        localIP += localIPChar;
      }
      Serial.print("Local WiFi IP: ");
      Serial.println(localIP);
    }
  }
}

void getDateTimeFromESP() {
  String dateTimeStr = "";  // Variable to store received date and time string
  while (esp8266.available() > 0) {
    char receivedChar = esp8266.read();
    if (receivedChar == '\n') {
      break;  // Break the loop when newline character is received
    }
    dateTimeStr += receivedChar;  // Append received character to the string
  }

  Serial.print("Date and Time from ESP8266: ");
  Serial.println(dateTimeStr);

  // Extract date and time components
  int year, month, day, hour, minute, second;
  sscanf(dateTimeStr.c_str(), "%d/%d/%d %d:%d:%d",
         &year, &month, &day, &hour, &minute, &second);
  Serial.print("Parsed Date and Time: ");
  // Serial.print("Year: ");
  // Serial.print(year);
  // Serial.print(", Month: ");
  // Serial.print(month);
  // Serial.print(", Day: ");
  // Serial.print(day);
  // Serial.print(", Hour: ");
  // Serial.print(hour);
  // Serial.print(", Minute: ");
  // Serial.print(minute);
  // Serial.print(", Second: ");
  // Serial.println(second);

  // Set RTC with received date and time
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.println("RTC synchronized with ESP8266 time.");
}

void Sensors_initialization() {
  Wire.begin();
  lightMeter.begin();
  mlx.begin();
  dht.begin();
  sensorsBoard.begin();
  //sensorsBoard.setResolution(RESOLUTION_DALLAS);
}

float loggingTemperature() {
  /**/
  h = dht.readHumidity();
  t = dht.readTemperature();  // Read temperature as Celsius
  if (isnan(h) || isnan(t)) {
    h = 0;
    t = 0;
  }
  h2 = dht2.readHumidity();
  t2 = dht2.readTemperature();  // Read temperature as Celsius
  if (isnan(h2) || isnan(t2)) {
    h2 = 0;
    t2 = 0;
  }
  h3 = dht3.readHumidity();
  t3 = dht3.readTemperature();  // Read temperature as Celsius
  if (isnan(h3) || isnan(t3)) {
    h3 = 0;
    t3 = 0;
  }
  /* Measuring temperature from NTC probes*/
  loggingNTC();
  /*Measuring temperature on board*/
  sensorsBoard.requestTemperatures();
  tempBoard = sensorsBoard.getTempCByIndex(0);
  /*Measuring the irradiance*/
  uint16_t lux = lightMeter.readLightLevel();
  irr = (lux * 0.0079) * 2.2;
  /*Measuring the temperature of ambient and sky using the MLX*/
  IR_temp_amb = mlx.readAmbientTempC();
  if (isnan(IR_temp_amb)) {
    IR_temp_amb = 0;
  }
  IR_temp_sky = mlx.readObjectTempC();
  if (isnan(IR_temp_sky)) {
    IR_temp_sky = 0;
  }
  //updateData();
  /* String Writing the String for the ESP8266*/
  str = String(t) + String(",") + String(h) + String(",") + String(t2) + String(",") +
        String(h2) + String(",") + String(t3) + String(",") + String(h3) + String(",") + 
        String(T[0]) + String(",") + String(T[1]) + String(",") + String(T[2]) + String(",") +
        String(T[3]) + String(",") + String(T[4]) + String(",") + String(tempBoard) +
        String(",") + String(irr) + String(",") + String(IR_temp_amb) + String(",") +
        String(IR_temp_sky);
}

void loggingNTC() {
  for (int i = 0; i < numChannels; ++i) {
    VRT[i] = (5.00 / 1023.00) * analogRead(analogPins[i]);  // Acquisition analog value of VRT
    VR[i] = VCC - VRT[i];
    RT[i] = VRT[i] / (VR[i] / R);  // Resistance of RT
    ln[i] = log(RT[i] / RT0);
    T[i] = (1 / ((ln[i] / B) + (1 / T0)));  // Temperature from thermistor
    T[i] -= 273.15;  // Conversion to Celsius
  }
}

/*Saving data on SD card file and print data on the terminal*/
void datalog() {
  DateTime now = rtc.now();  //HAVE TO BE SOSTITUTED WITH THE NTP TIME
  Date = String(now.day(), DEC) + String("/") + String(now.month(), DEC) + String("/") + String(now.year(), DEC);
  Time = String(now.hour(), DEC) + String(":") + String(now.minute(), DEC) + String(":") + String(now.second(), DEC);
  Date_Time = String(Date) + String(" ") + String(Time);
  loggingTemperature();

  str0 = String(Date_Time) + String(",") + String(str);
  appendData(str0);
  Serial.println(str0);
}

void appendData(String dataValue) {
  String filename = getTodayFilename();
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    // Append the calculated value to the file
    dataFile.println(dataValue);
    dataFile.close();
  } else {
    Serial.println("Error opening file: " + filename);
  }
}

void getstring() {
  esp8266.println(str);  //the string containing the data is sent to the ESP for the web and dashboard
}

unsigned long startTime = 0;
unsigned long interval = 0;
const unsigned long displayInterval = 2000;  // Adjust the interval as needed
int displayState = 0;                        // Initial state

const unsigned char plug[] PROGMEM = {
  // 'Pictogrammers-Material-Power-plug, 16x16px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x3f, 0xe0, 0x3f, 0xfc, 0x07, 0xfc,
  0x07, 0xfc, 0x3f, 0xfc, 0x3f, 0xe0, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 'Icons8-Windows-8-Industry-Processor', 16x16px
const unsigned char cpu_icon[] PROGMEM = {
  0x0a, 0x50, 0x00, 0x00, 0x3f, 0xfc, 0x2f, 0xfc, 0xbf, 0xfd, 0x3f, 0xfc, 0xbf, 0xfd, 0x3f, 0xfc,
  0x3f, 0xfc, 0xbf, 0xfd, 0x3f, 0xfc, 0xbf, 0xfd, 0x3f, 0xf4, 0x3f, 0xfc, 0x00, 0x00, 0x0a, 0x50
};

// 'Pictogrammers-Material-Sd', 16x16px
const unsigned char sd_icon[] PROGMEM = {
  0x00, 0x00, 0x01, 0xf8, 0x07, 0xf8, 0x0e, 0xa8, 0x1e, 0xa8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8,
  0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x00, 0x00
};

// 'Custom-Icon-Design-Mono-General-3-Wifi', 16x16px
const unsigned char Wifi_icon[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x1f, 0xf8, 0x70, 0x0e, 0xe0, 0x07, 0x07, 0xe0, 0x1e, 0x78,
  0x18, 0x18, 0x03, 0xc0, 0x07, 0xe0, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00
};

const unsigned char acs_point[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char empty_icon[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char Temperature_2[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x07, 0x80, 0x1f, 0x00,
  0x0c, 0x80, 0x3f, 0x80, 0x08, 0x80, 0x7f, 0x80, 0x0c, 0x80, 0x7f, 0xc0, 0x0f, 0x80, 0xff, 0xe0,
  0x0f, 0x81, 0xff, 0xe0, 0x0f, 0x81, 0xff, 0xf0, 0x0f, 0x83, 0xff, 0xf0, 0x0f, 0x83, 0xff, 0xf8,
  0x0f, 0x83, 0xff, 0xf8, 0x0f, 0x87, 0xff, 0xf8, 0x0f, 0xc7, 0xff, 0xf8, 0x1f, 0xc7, 0xff, 0xf8,
  0x1f, 0xe3, 0xff, 0xf8, 0x1f, 0xe3, 0xff, 0xf8, 0x1f, 0xc3, 0xff, 0xf0, 0x1f, 0xc1, 0xff, 0xf0,
  0x0f, 0x80, 0xff, 0xe0, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char Therm_icon[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00,
  0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0xff, 0xc0, 0x00, 0xcc, 0xff, 0xc0,
  0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0xff, 0x00, 0x00, 0xcc, 0xff, 0x00,
  0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0xff, 0xc0, 0x00, 0xcc, 0xff, 0xc0,
  0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0xff, 0x00, 0x00, 0xcc, 0xff, 0x00,
  0x00, 0xcc, 0x00, 0x00, 0x00, 0xcc, 0x00, 0x00, 0x01, 0x86, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00,
  0x03, 0x33, 0x00, 0x00, 0x03, 0x33, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x01, 0x86, 0x00, 0x00,
  0x01, 0xfe, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char Sun_icon[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x0c, 0x0e, 0x00, 0x00, 0x08, 0x02, 0x00, 0x00,
  0x10, 0x01, 0x00, 0x00, 0x30, 0x03, 0x80, 0x00, 0x30, 0x1f, 0xe0, 0x00, 0x10, 0x3f, 0xf0, 0x00,
  0x10, 0x7f, 0xff, 0x00, 0x30, 0xff, 0xff, 0xc0, 0x10, 0xff, 0xff, 0xe0, 0x09, 0xff, 0xff, 0xf0,
  0x0d, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xfc,
  0x1f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xf8, 0x1f, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xe0,
  0x07, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char Date_icon[] PROGMEM = {
  0x00, 0xe0, 0x06, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x01, 0xf0, 0x0f, 0x00,
  0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xe0, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfc,
  0x7f, 0xff, 0xff, 0xfc, 0x7f, 0x9f, 0xff, 0xfc, 0x7e, 0x6f, 0xff, 0xfc, 0x7f, 0xf7, 0xff, 0xfc,
  0x7b, 0xf7, 0xff, 0xfc, 0x7b, 0xf7, 0xff, 0xfc, 0x79, 0xf7, 0xff, 0xfc, 0x7c, 0xe7, 0xff, 0xfc,
  0x7e, 0x1f, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Drawing the screen

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  u8g.drawBitmapP(0, 0, 2, 16, plug);
  if (clock_status != "Fail")
    u8g.drawBitmapP(20, 0, 2, 16, cpu_icon);
  else if (clock_status == "Fail")
    u8g.drawBitmapP(20, 0, 2, 16, empty_icon);
  if (sd_status == "Done")
    u8g.drawBitmapP(40, 0, 2, 16, sd_icon);
  else if (sd_status == "Fail")
    u8g.drawBitmapP(50, 0, 2, 16, empty_icon);
  if (wifi_status == "Done")
    u8g.drawBitmapP(60, 0, 2, 16, Wifi_icon);
  else if (wifi_status == "Fail")
    u8g.drawBitmapP(50, 0, 2, 16, acs_point);
  u8g.setFont(u8g_font_8x13);
  u8g.setPrintPos(78, 13);
  u8g.print("FRESCO");
}

void drawAIP() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Acc Pnt IP");
    u8g.setPrintPos(10, 50);
    u8g.print(apIP);
  } while (u8g.nextPage());
}

void drawLIP() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Wi-Fi loc IP");
    u8g.setPrintPos(10, 50);
    u8g.print(localIP);
  } while (u8g.nextPage());
}

void Draw_dateandtime() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Date & Time");
    u8g.setPrintPos(0, 45);
    u8g.print(Date);
    u8g.setPrintPos(0, 60);
    u8g.print(Time);
    u8g.drawBitmapP(90, 25, 4, 32, Date_icon);
  } while (u8g.nextPage());
}

void Draw_th1() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Tamb & Hum 1");
    u8g.setPrintPos(5, 40);
    u8g.print(t);
    u8g.setPrintPos(45, 40);
    u8g.print("C");
    u8g.setPrintPos(10, 60);
    u8g.print(h);
    u8g.setPrintPos(55, 60);
    u8g.print("%");
    u8g.drawBitmapP(90, 25, 4, 32, Temperature_2);
  } while (u8g.nextPage());
}

void Draw_th2() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Tamb & Hum 2");
    u8g.setPrintPos(5, 40);
    u8g.print(t2);
    u8g.setPrintPos(45, 40);
    u8g.print("C");
    u8g.setPrintPos(10, 60);
    u8g.print(h2);
    u8g.setPrintPos(55, 60);
    u8g.print("%");
    u8g.drawBitmapP(90, 25, 4, 32, Temperature_2);
  } while (u8g.nextPage());
}

void Draw_th3() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Tamb & Hum 3");
    u8g.setPrintPos(5, 40);
    u8g.print(t3);
    u8g.setPrintPos(45, 40);
    u8g.print("C");
    u8g.setPrintPos(10, 60);
    u8g.print(h3);
    u8g.setPrintPos(55, 60);
    u8g.print("%");
    u8g.drawBitmapP(90, 25, 4, 32, Temperature_2);
  } while (u8g.nextPage());
}

void Draw_t1t2() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Tsample 1 and 2");
    u8g.setPrintPos(5, 40);
    u8g.print(T[0], 2);
    u8g.setPrintPos(65, 40);
    u8g.print("C");
    u8g.setPrintPos(10, 60);
    u8g.print(T[1], 2);
    u8g.setPrintPos(75, 60);
    u8g.print("C");
    u8g.drawBitmapP(90, 35, 4, 32, Therm_icon);
  } while (u8g.nextPage());
}

void Draw_t3t4() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Tsample 3 and 4");
    u8g.setPrintPos(5, 40);
    u8g.print(T[2], 2);
    u8g.setPrintPos(65, 40);
    u8g.print("C");
    u8g.setPrintPos(10, 60);
    u8g.print(T[3], 2);
    u8g.setPrintPos(75, 60);
    u8g.print("C");
    u8g.drawBitmapP(90, 35, 4, 32, Therm_icon);
  } while (u8g.nextPage());
}

void Draw_tBxtBr() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("TBox and Board");
    u8g.setPrintPos(5, 40);
    u8g.print(T[4], 2);
    u8g.setPrintPos(65, 40);
    u8g.print("C");
    u8g.setPrintPos(10, 60);
    u8g.print(tempBoard, 2);
    u8g.setPrintPos(75, 60);
    u8g.print("C");
    u8g.drawBitmapP(90, 35, 4, 32, Therm_icon);
  } while (u8g.nextPage());
}

void Draw_TSirr() {
  u8g.firstPage();  // first page
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(3, 30);
    u8g.print("Irr and SkyT");
    u8g.setPrintPos(5, 40);
    u8g.print(irr, 1);
    u8g.setPrintPos(65, 40);
    u8g.print("W/m2");
    u8g.setPrintPos(10, 60);
    u8g.print(IR_temp_sky, 2);
    u8g.setPrintPos(75, 60);
    u8g.print("C");
    u8g.drawBitmapP(90, 35, 4, 32, Sun_icon);
  } while (u8g.nextPage());
}

void init_screen() {
  u8g.firstPage();  // first page
  do {
    draw();
  } while (u8g.nextPage());
  startTime = millis();  // Record the start time
  while (millis() - startTime < 2000) {
    drawAIP();
  }
  startTime = millis();  // Reset the start time
  while (millis() - startTime < 2000) {
    drawLIP();
  }
  u8g.firstPage();  // first page
  do {
  } while (u8g.nextPage());
}

void displaying_data(void) {
  switch (displayState) {
    case 0:
      // Display general information
      u8g.firstPage();
      do {
        draw();
      } while (u8g.nextPage());
      break;
    case 1:
      // Display date and time
      Draw_dateandtime();
      break;
    case 2:
      // Display temperature and humidity 1
      Draw_th1();
      break;
    case 3:
      // Display temperature and humidity 2
      Draw_th2();
      break;
    case 4:
      // Display temperature and humidity 2
      Draw_th3();
      break;
    case 5:
      // Display temperature and humidity 2
      Draw_t1t2();
      break;
    case 6:
      // Display temperature and humidity 2
      Draw_t3t4();
      break;
    case 7:
      // Display temperature and humidity 2
      Draw_tBxtBr();
      break;
    case 8:
      // Display temperature and humidity 2
      Draw_TSirr();
      break;
    default:
      // Handle unknown state
      break;
  }

  // Check if it's time to switch to the next state
  if (millis() - startTime >= displayInterval) {
    startTime = millis();  // Reset the start time
    displayState++;

    // Reset state to the initial state if we've reached the end
    if (displayState >= 9) {
      displayState = 0;
    }
  }
  //Serial.print("State: ");
  //Serial.println(displayState);
}
