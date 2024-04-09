//#include "secrets.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#define DEVICE "ESP8266"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <time.h>                   // for time() ctime()
// Time zone info
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
//#define MY_NTP_SERVER "at.pool.ntp.org"      
#define MY_NTP_SERVER "ntp.unifi.it" 
time_t now;                         // this are the seconds since Epoch (1970) - UTC
tm tm;                              // the structure tm holds time information in a more convenient way      
// Define the array to store the data
const int array_length = 15; // Adjust this based on your expected number of elements
String array[array_length];
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "INFLUXDB_URL";
const char* PARAM_INPUT_4 = "INFLUXDB_DB_NAME";
const char* PARAM_INPUT_5 = "INFLUXDB_ORG";
const char* PARAM_INPUT_6 = "INFLUXDB_BUCKET";
const char* PARAM_INPUT_7 = "INFLUXDB_TOKEN";

bool useCloudInfluxDB = false;
/* SSID and PASSWORD for the Acces Point*/
const char* ssid1 = "FRESCO-board";
const char* password1 = "123456789";

String ssid;
String pass;
String INFLUXDB_URL;
String INFLUXDB_DB_NAME;
String INFLUXDB_ORG;
String INFLUXDB_BUCKET;
String INFLUXDB_TOKEN;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* INFLUXDB_URLPath = "/INFLUXDB_URL.txt";
const char* INFLUXDB_DB_NAMEPath = "/INFLUXDB_DB_NAME.txt";
const char* INFLUXDB_ORGPath = "/INFLUXDB_ORG.txt";
const char* INFLUXDB_BUCKETPath = "/INFLUXDB_BUCKET.txt";
const char* INFLUXDB_TOKENPath = "/INFLUXDB_TOKEN.txt";

boolean restart = false;

// Create an Event Source on /events
AsyncEventSource events("/events");
AsyncEventSource events2("/events2");
AsyncEventSource events3("/events3");
AsyncEventSource events4("/events4");

JSONVar readings;
JSONVar readings2;
JSONVar readings3;
JSONVar readings4;

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    //Serial.println("An error has occurred while mounting LittleFS");
  }
  else {
    //Serial.println("LittleFS mounted successfully");
  }
}

String readFile(fs::FS &fs, const char * path) {
  //Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    //Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  file.close();
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  //Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    //Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    //Serial.println("- file written");
  } else {
    //Serial.println("- frite failed");
  }
  file.close();
}

// InfluxDB client instance for InfluxDB //See secrets for the Influx DB URL and DB Name
InfluxDBClient client;
//Define Data point
Point sensor("Measurements");

// Initialize WiFi
bool initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid1, password1);

  // Wait for the softAP to start
  delay(1000);

  // Connect to the WiFi network
  WiFi.begin(ssid.c_str(), pass.c_str());

  //Serial.println("Connecting to WiFi...");

  // Wait for the WiFi connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    //Serial.print(".");
    attempts++;
  }
  //WiFi.setAutoReconnect(true);
  //WiFi.persistent(true);

  if (WiFi.status() == WL_CONNECTED) {
    // Send 'C' to indicate connected status
    Serial.write('C');
    Serial.println(WiFi.softAPIP());
    Serial.println(WiFi.localIP());
    showTime();
    return true;
  } else {
    // Send 'D' to indicate disconnected status
    Serial.write('D');
    Serial.println(WiFi.softAPIP());
    Serial.println(WiFi.localIP());
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  initLittleFS();
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  INFLUXDB_URL = readFile(LittleFS, INFLUXDB_URLPath);
  INFLUXDB_DB_NAME = readFile(LittleFS, INFLUXDB_DB_NAMEPath);
  INFLUXDB_ORG = readFile(LittleFS, INFLUXDB_ORGPath); // New
  INFLUXDB_BUCKET = readFile(LittleFS, INFLUXDB_BUCKETPath); // New
  INFLUXDB_TOKEN = readFile(LittleFS, INFLUXDB_TOKENPath); // New
//  Serial.println(ssid);
//  Serial.println(pass);
//  Serial.println(INFLUXDB_URL);
//  Serial.println(INFLUXDB_DB_NAME);
//  Serial.println(INFLUXDB_ORG);
//  Serial.println(INFLUXDB_BUCKET);
//  Serial.println(INFLUXDB_TOKEN);
  configTime(TZ_INFO, MY_NTP_SERVER);
  initWiFi();
  if (!INFLUXDB_ORG.isEmpty() && !INFLUXDB_BUCKET.isEmpty() && !INFLUXDB_TOKEN.isEmpty()) {
  client.setConnectionParams(INFLUXDB_URL.c_str(), INFLUXDB_ORG.c_str(), INFLUXDB_BUCKET.c_str(), INFLUXDB_TOKEN.c_str(), InfluxDbCloud2CACert);
} else {
  // Set local connection params with default values for missing parameters
  //client.setConnectionParams(INFLUXDB_URL.c_str(), INFLUXDB_DB_NAME.c_str());
  client.setConnectionParamsV1(INFLUXDB_URL.c_str(), INFLUXDB_DB_NAME.c_str());
}

  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  //timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  delay(2000);
  timeSync(TZ_INFO, "ntp.unifi.it");

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");
  //server.begin();

  String ipString = "ESP IP: " + WiFi.localIP().toString() + "  Access Point IP: " + WiFi.softAPIP().toString();

  server.on("/getIPs", HTTP_GET, [ipString](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", ipString);
  });

  // Web Server Root URL for WiFi Configuration
  server.on("/wifimanager", HTTP_GET, handleWiFiConfiguration);
  server.on("/wifimanager", HTTP_POST, handleWiFiConfiguration);
  server.on("/database", HTTP_GET, handleInfluxDBConfiguration);
  server.on("/database", HTTP_POST, handleInfluxDBConfiguration);

  //server.serveStatic("/", LittleFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest * request) {
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });

  server.on("/readings2", HTTP_GET, [](AsyncWebServerRequest * request) {
    String json2 = getSensorReadings2();
    request->send(200, "application/json2", json2);
    json2 = String();
  });

  server.on("/readings3", HTTP_GET, [](AsyncWebServerRequest * request) {
    String json3 = getSensorReadings3();
    request->send(200, "application/json3", json3);
    json3 = String();
  });

  server.on("/readings4", HTTP_GET, [](AsyncWebServerRequest * request) {
    String json4 = getSensorReadings4();
    request->send(200, "application/json4", json4);
    json4 = String();
  });

  events.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      //Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events);

  events2.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      // Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events2);

  events3.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      // Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events3);

  events4.onConnect([](AsyncEventSourceClient * client) {
    if (client->lastId()) {
      // Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 1000);
  });
  server.addHandler(&events4);

  // Start server
  server.begin();

  // Add constant tags - only once
  sensor.addTag("device", DEVICE);
}

//int starttime = millis();

void loop() {
  if (restart) {
    delay(2000);
    ESP.restart();
  }

  static bool reading = false;
  static int q = 0;

  while (Serial.available() > 0) {
    char received = Serial.read();

    if (received == ' ' || received == ',' || received == '\n') {
      if (reading) {
        reading = false;
        q++;
      }
    } else {
      if (!reading) {
        reading = true;
        array[q] = received;
      } else {
        array[q] += received;
      }
    }
    if (q == array_length) {
      printArray(array, array_length);
      q = 0;

      /* Set fields in InfluxDB Database */
      sensor.clearFields();                                         // Clear old data in fields
      sensor.addField("Temp_amb", array[0].toFloat());              // Set Temp_amb value
      //Serial.print("Writing: ");
      //Serial.println(sensor.toLineProtocol());
      sensor.addField("Humidity", array[1].toFloat());              // Set humidity value
      sensor.addField("Temp_amb 2", array[2].toFloat());              // Set Temp_amb value
      sensor.addField("Humidity 2", array[3].toFloat());              // Set humidity value
      sensor.addField("Temp_amb 3", array[4].toFloat());              // Set Temp_amb value
      sensor.addField("Humidity 3", array[5].toFloat());              // Set humidity value
      sensor.addField("Temp_S1", array[6].toFloat());               // Set Temp_Sample1 value
      sensor.addField("Temp_S2", array[7].toFloat());               // Set Temp_Sample2 value
      sensor.addField("Temp_S3", array[8].toFloat());               // Set Temp_Sample3 value
      sensor.addField("Temp_S4", array[9].toFloat());               // Set Temp_Sample4 value
      sensor.addField("Temp_Box", array[10].toFloat());              // Set Temp_box value
      sensor.addField("Temp_Board", array[11].toFloat());            // Set Temp_board value
      sensor.addField("Irradiance", array[12].toFloat());            // Set Irradiance value
      sensor.addField("IR Amb", array[13].toFloat());                // Set IR amb value
      sensor.addField("IR Sky", array[14].toFloat());               // Set IR Sky value
      /*
        sensor.addField("TPC1", array[15].toFloat());               // temp PCool sample 1
        sensor.addField("PD1", array[16].toFloat());               // PD PCool sample 1
        sensor.addField("TPC2", array[17].toFloat());               // temp PCool sample 2
        sensor.addField("PD2", array[18].toFloat());               // PD PCool sample 2
        sensor.addField("SetP", array[19].toFloat());               // Temp set point for PCool
      */
      client.writePoint(sensor);                                    // Push data to InfluxDB
      sensor.clearFields();
      /**/
      // Send Events to the client with the Sensor Readings Every 10 seconds
      events.send("ping", NULL, millis());
      events.send(getSensorReadings().c_str(), "new_readings" , millis());
      events2.send(getSensorReadings2().c_str(), "new_readings2" , millis());
      //events3.send(getSensorReadings3().c_str(), "new_readings3" , millis());
      //events4.send(getSensorReadings4().c_str(), "new_readings4" , millis());
    }
  }
}

void printArray(String arr[], int size) {
  Serial.print("[");
  for (int i = 0; i < size; i++) {
    Serial.print(arr[i].toFloat());
    if (i < size - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
}

String getSensorReadings() {
  readings["Temp_amb"] = array[0];
  readings["Temp_amb 2"] = array[2];
  readings["Box T DHT"] = array[4];
  readings["Temp_S1"] = array[6];
  readings["Temp_S2"] = array[7];
  readings["Temp_S3"] = array[8];
  readings["Temp_S4"] = array[9];
  readings["Temp_Box"] = array[10];
  readings["IR Sky"] = array[14];
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

String getSensorReadings2() {
  readings2["Humidity"] = array[1];
  readings2["Humidity 2"] = array[3];
  readings2["Box H DHT"] = array[5];
  readings2["Irradiance"] = array[12];
  String jsonString = JSON.stringify(readings2);
  return jsonString;
}

String getSensorReadings3() {
  readings3["Temp_amb"] = array[0];
  readings3["TPC1"] = array[2];
  readings3["TPC2"] = array[4];
  readings3["Temp_S1"] = array[6];
  readings3["Temp_S2"] = array[7];
  String jsonString = JSON.stringify(readings3);
  return jsonString;
}

String getSensorReadings4() {
  readings4["PD1"] = array[1];
  readings4["PD2"] = array[3];
  String jsonString = JSON.stringify(readings4);
  return jsonString;
}


void handleWiFiConfiguration(AsyncWebServerRequest * request) {
  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isPost()) {
      if (p->name() == PARAM_INPUT_1) {
        ssid = p->value().c_str();
        Serial.print("SSID set to: ");
        Serial.println(ssid);
        writeFile(LittleFS, ssidPath, ssid.c_str());
      }
      if (p->name() == PARAM_INPUT_2) {
        pass = p->value().c_str();
        Serial.print("Password set to: ");
        Serial.println(pass);
        writeFile(LittleFS, passPath, pass.c_str());
      }
    }
  }
  restart = true;
  request->send(200, "text/plain", "Done. ESP will restart, connect to your ssid: " + ssid);
}

void handleInfluxDBConfiguration(AsyncWebServerRequest * request) {
  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isPost()) {
      if (p->name() == PARAM_INPUT_3) {
        INFLUXDB_URL = p->value().c_str();
        Serial.print("INFLUXDB_URL set to: ");
        Serial.println(INFLUXDB_URL);
        writeFile(LittleFS, INFLUXDB_URLPath, INFLUXDB_URL.c_str());
      }
      if (p->name() == PARAM_INPUT_4) {
        INFLUXDB_DB_NAME = p->value().c_str();
        Serial.print("INFLUXDB_DB_NAME set to: ");
        Serial.println(INFLUXDB_DB_NAME);
        writeFile(LittleFS, INFLUXDB_DB_NAMEPath, INFLUXDB_DB_NAME.c_str());
      }
      if (p->name() == PARAM_INPUT_5) {
        INFLUXDB_ORG = p->value().c_str();
        Serial.print("INFLUXDB_ORG set to: ");
        Serial.println(INFLUXDB_ORG);
        writeFile(LittleFS, INFLUXDB_ORGPath, INFLUXDB_ORG.c_str());
      }
      if (p->name() == PARAM_INPUT_6) {
        INFLUXDB_BUCKET = p->value().c_str();
        Serial.print("INFLUXDB_BUCKET set to: ");
        Serial.println(INFLUXDB_BUCKET);
        writeFile(LittleFS, INFLUXDB_BUCKETPath, INFLUXDB_BUCKET.c_str());
      }
      if (p->name() == PARAM_INPUT_7) {
        INFLUXDB_TOKEN = p->value().c_str();
        Serial.print("INFLUXDB_TOKEN set to: ");
        Serial.println(INFLUXDB_TOKEN);
        writeFile(LittleFS, INFLUXDB_TOKENPath, INFLUXDB_TOKEN.c_str());
      }
    }
  }
  restart = true;
  request->send(200, "text/plain", "Done. ESP will restart, connect to INFLUXDB_URL: " + INFLUXDB_URL);
}


void showTime() {
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time
  Serial.print(tm.tm_year + 1900);  // years since 1900
  Serial.print("/");
  Serial.print(tm.tm_mon + 1);      // January = 0 (!)
  Serial.print("/");
  Serial.print(tm.tm_mday);         // day of month
  Serial.print(" ");
  Serial.print(tm.tm_hour);         // hours since midnight  0-23
  Serial.print(":");
  Serial.print(tm.tm_min);          // minutes after the hour  0-59
  Serial.print(":");
  Serial.print(tm.tm_sec);          // seconds after the minute  0-61*
if (tm.tm_isdst == 1)             // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
}
