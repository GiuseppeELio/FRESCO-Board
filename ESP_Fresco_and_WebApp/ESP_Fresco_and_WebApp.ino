//#include "secrets.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wpa2_enterprise.h"
#define DEVICE "ESP8266"
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <time.h>                   // for time() ctime()

// Time zone info
//#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
//#define MY_NTP_SERVER "at.pool.ntp.org"
//#define MY_NTP_SERVER "ntp.unifi.it"
time_t now;                         // this are the seconds since Epoch (1970) - UTC
tm tm;                              // the structure tm holds time information in a more convenient way
// Define the array to store the data
const int MAX_ARRAY_LENGTH = 20;
// Declare global variables
int array_length = 0;  // Variable to store the length of the array
String array[MAX_ARRAY_LENGTH]; // Define array with maximum length
bool isPCool = false; // Flag to indicate whether it's PCool or Tdrop
/**/
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
const char* PARAM_INPUT_8 = "ntp";
const char* PARAM_INPUT_9 = "tz_info";
const char* PARAM_INPUT_10 = "array_length";
const char* PARAM_INPUT_11 = "ntc_b_value";
const char* PARAM_INPUT_12 = "data_transfer_time";
const char* PARAM_INPUT_13 = "saving_time";
const char* PARAM_INPUT_14 = "displaying_time";
const char* PARAM_INPUT_15 = "irr_cal";
const char* PARAM_INPUT_16 = "pid_set_point";
const char* PARAM_INPUT_17 = "sample_surface";
const char* PARAM_INPUT_18 = "academicSsid";
const char* PARAM_INPUT_19 = "academicUser";
const char* PARAM_INPUT_20 = "academicPass";

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
String NTP_server;
String TZ_INFO;
int ArrayLength;
String ntc_b_value;
String data_transfer_time;
String saving_time;
String displaying_time;
String irr_cal;
String pid_set_point;
String sample_surface;
String academicSsid;
String academicUser;
String academicPass;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* INFLUXDB_URLPath = "/INFLUXDB_URL.txt";
const char* INFLUXDB_DB_NAMEPath = "/INFLUXDB_DB_NAME.txt";
const char* INFLUXDB_ORGPath = "/INFLUXDB_ORG.txt";
const char* INFLUXDB_BUCKETPath = "/INFLUXDB_BUCKET.txt";
const char* INFLUXDB_TOKENPath = "/INFLUXDB_TOKEN.txt";
const char* NTP_server_Path = "/NTP_server.txt";
const char* TZ_INFO_Path = "/TZ_INFO.txt";
const char* ArrayLength_Path = "/ArrayLength.txt";
const char* ntc_b_value_Path = "/ntc_b_value.txt";
const char* data_transfer_time_Path = "/data_transfer_time.txt";
const char* saving_time_Path = "/saving_time.txt";
const char* displaying_time_Path = "/displaying_time.txt";
const char* irr_cal_Path = "/irr_cal.txt";
const char* pid_set_point_Path = "/pid_set_point.txt";
const char* sample_surface_Path = "/sample_surface.txt";

const char* academicSsid_Path = "/academicSsid.txt";
const char* academicUser_Path = "/academicUser.txt";
const char* academicPass_Path = "/academicPass.txt";

boolean restart = false;
bool sensorState = false; // Global variable to hold the sensor state

// Create an Event Source on /events
AsyncEventSource events("/events");
AsyncEventSource events2("/events2");
AsyncEventSource events3("/events3");
AsyncEventSource events4("/events4");

JSONVar readings;
JSONVar readings2;
JSONVar readings3;
JSONVar readings4;
JSONVar DynamicJsonDocument;

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
  if (!academicUser.isEmpty() && !academicPass.isEmpty()) {
    // Connect to the WiFi network
    wifi_set_opmode(STATION_MODE);
    struct station_config wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    //strcpy((char *)wifi_config.ssid, ssid);
    strcpy((char *)wifi_config.ssid, academicSsid.c_str());
    wifi_station_set_config(&wifi_config);
    // DISABLE authentication using certificates - But risk leaking your password to someone claiming to be "eduroam"
    wifi_station_clear_cert_key();
    wifi_station_clear_enterprise_ca_cert();
    // Authenticate using username/password
    wifi_station_set_wpa2_enterprise_auth(1);
    wifi_station_set_enterprise_identity((uint8 *)academicUser.c_str(), strlen(academicUser.c_str()));
    wifi_station_set_enterprise_username((uint8 *)academicUser.c_str(), strlen(academicUser.c_str()));
    wifi_station_set_enterprise_password((uint8 *)academicPass.c_str(), strlen(academicPass.c_str()));

    wifi_station_connect();
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      //Serial.print(".");
      attempts++;
    }
  } else {
    WiFi.begin(ssid.c_str(), pass.c_str());

    //Serial.println("Connecting to WiFi...");

    // Wait for the WiFi connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      //Serial.print(".");
      attempts++;
    }
  }
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid1, password1);
  // Wait for the softAP to start
  delay(1000);
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
  NTP_server = readFile(LittleFS, NTP_server_Path);
  TZ_INFO = readFile(LittleFS, TZ_INFO_Path);
  String arrayLengthStr = readFile(LittleFS, ArrayLength_Path);
  ArrayLength = arrayLengthStr.toInt();
  ntc_b_value = readFile(LittleFS, ntc_b_value_Path);
  data_transfer_time = readFile(LittleFS, data_transfer_time_Path);
  saving_time = readFile(LittleFS, saving_time_Path);
  displaying_time = readFile(LittleFS, displaying_time_Path);
  irr_cal = readFile(LittleFS, irr_cal_Path);
  pid_set_point = readFile(LittleFS, pid_set_point_Path);
  sample_surface = readFile(LittleFS, sample_surface_Path);

  academicSsid = readFile(LittleFS, academicSsid_Path);
  academicUser = readFile(LittleFS, academicUser_Path);
  academicPass = readFile(LittleFS, academicPass_Path);

  //  Serial.println(ssid);
  //  Serial.println(pass);
  //  Serial.println(INFLUXDB_URL);
  //  Serial.println(INFLUXDB_DB_NAME);
  //  Serial.println(INFLUXDB_ORG);
  //  Serial.println(INFLUXDB_BUCKET);
  //  Serial.println(INFLUXDB_TOKEN);
  //  Serial.println(NTP_server);
  //  Serial.println(TZ_INFO);
  //  Serial.println(ArrayLength);
  String paramString = ntc_b_value + "/" + data_transfer_time + "/" + saving_time + "/" + displaying_time + "/" + irr_cal + "/" + pid_set_point + "/" + sample_surface + "\n";
  // Send the string to Arduino
  Serial.print(paramString);
  //configTime(TZ_INFO, MY_NTP_SERVER);
  configTime(TZ_INFO.c_str(), NTP_server.c_str());
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
  timeSync(TZ_INFO.c_str(), NTP_server.c_str());

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

  // Web Server Root URL for WiFi, Database and Settings Configuration
  server.on("/wifimanager", HTTP_GET, handleWiFiConfiguration);
  server.on("/wifimanager", HTTP_POST, handleWiFiConfiguration);
  server.on("/database", HTTP_GET, handleInfluxDBConfiguration);
  server.on("/database", HTTP_POST, handleInfluxDBConfiguration);
  server.on("/settings", HTTP_GET, handleSettingsConfiguration);
  server.on("/settings", HTTP_POST, handleSettingsConfiguration);

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
  if (Serial.available() > 0) {
    // Read the data from serial port
    String receivedData = Serial.readStringUntil('\n');
    // Process the received data
    processReceivedData(receivedData);
  }
}

void processReceivedData(const String& data) {
  // Parse the received data and split it into elements
  int count = 0;
  int startIndex = 0;
  for (int i = 0; i <= data.length(); ++i) {  // Include the end of string
    if (i == data.length() || data[i] == ' ' || data[i] == ',' || data[i] == '\n') {
      array[count++] = data.substring(startIndex, i);
      startIndex = i + 1;
    }
  }

  // Process the received data based on its length
  if (count <= 15) {
    // TDrop data
    isPCool = false;
    //Serial.println("Received TDrop data:");
  } else {
    // PCool data
    //Serial.println("Received PCool data:");
    isPCool = true;
  }
  server.on("/status", HTTP_GET, handleStatus);
  printArray(array, count);

  // Populate sensor fields
  sensor.clearFields();  // Clear old data in fields
  for (int i = 0; i < count; ++i) {
    String fieldName = "Temp_amb";
    if (i == 1) fieldName = "Humidity";
    else if (i == 2) fieldName = "Temp_amb 2";
    else if (i == 3) fieldName = "Humidity 2";
    else if (i == 4) fieldName = "Temp_amb 3";
    else if (i == 5) fieldName = "Humidity 3";
    else if (i == 6) fieldName = "Temp_S1";
    else if (i == 7) fieldName = "Temp_S2";
    else if (i == 8) fieldName = "Temp_S3";
    else if (i == 9) fieldName = "Temp_S4";
    else if (i == 10) fieldName = "Temp_Box";
    else if (i == 11) fieldName = "Temp_Board";
    else if (i == 12) fieldName = "Irradiance";
    else if (i == 13) fieldName = "IR Amb";
    else if (i == 14) fieldName = "IR Sky";
    else if (i == 15) fieldName = "PD1";
    else if (i == 16) fieldName = "TPC1";
    else if (i == 17) fieldName = "PD2";
    else if (i == 18) fieldName = "TPC2";
    else if (i == 19) fieldName = "SetP";

    sensor.addField(fieldName, array[i].toFloat());
  }

  // Push data to InfluxDB
  client.writePoint(sensor);

  // Send events
  events.send("ping", NULL, millis());
  events.send(getSensorReadings().c_str(), "new_readings", millis());
  events2.send(getSensorReadings2().c_str(), "new_readings2", millis());
  if (isPCool) {
    events3.send(getSensorReadings3().c_str(), "new_readings3", millis());
    events4.send(getSensorReadings4().c_str(), "new_readings4", millis());
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
  readings3["TPC1"] = array[16];
  readings3["TPC2"] = array[18];
  readings3["Temp_S1"] = array[6];
  readings3["Temp_S2"] = array[7];
  String jsonString = JSON.stringify(readings3);
  return jsonString;
}

String getSensorReadings4() {
  readings4["PD1"] = array[15];
  readings4["PD2"] = array[17];
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
      if (p->name() == PARAM_INPUT_18) {
        academicSsid = p->value().c_str();
        Serial.print("Aca SSDI set to: ");
        Serial.println(academicSsid);
        writeFile(LittleFS, academicSsid_Path, academicSsid.c_str());
      }
      if (p->name() == PARAM_INPUT_19) {
        academicUser = p->value().c_str();
        Serial.print("Aca Username set to: ");
        Serial.println(academicUser);
        writeFile(LittleFS, academicUser_Path, academicUser.c_str());
      }
      if (p->name() == PARAM_INPUT_20) {
        academicPass = p->value().c_str();
        Serial.print("Aca Password set to: ");
        Serial.println(academicPass);
        writeFile(LittleFS, academicPass_Path, academicPass.c_str());
      }
    }
  }
  restart = true;
  request->send(200, "text/plain", "Done. ESP will restart, connect to your ssid: " + ssid + "or accademic" + academicSsid);
  Serial.println("RestartArduino");
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
  Serial.println("RestartArduino");
}

// Handle settings configuration
void handleSettingsConfiguration(AsyncWebServerRequest * request) {
  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isPost()) {
      if (p->name() == PARAM_INPUT_8) {
        NTP_server = p->value().c_str();
        Serial.print("NTP server set to: ");
        Serial.println(NTP_server);
        // Update LittleFS file
        writeFile(LittleFS, NTP_server_Path, NTP_server.c_str());
      }
      if (p->name() == PARAM_INPUT_9) {
        TZ_INFO = p->value().c_str();
        Serial.print("TZ info set to: ");
        Serial.println(TZ_INFO);
        // Update LittleFS file
        writeFile(LittleFS, TZ_INFO_Path, TZ_INFO.c_str());
      }
      if (p->name() == PARAM_INPUT_10) {
        ArrayLength = p->value().toInt();
        Serial.print("Array length set to: ");
        Serial.println(ArrayLength);
        // Update LittleFS file
        writeFile(LittleFS, ArrayLength_Path, String(ArrayLength).c_str());
      }
      if (p->name() == PARAM_INPUT_11) {
        ntc_b_value = p->value().c_str();
        Serial.print("ntc_b_value set to: ");
        Serial.println(ntc_b_value);
        // Update LittleFS file
        writeFile(LittleFS, ntc_b_value_Path, ntc_b_value.c_str());
      }
      if (p->name() == PARAM_INPUT_12) {
        data_transfer_time = p->value().c_str();
        Serial.print("data_transfer_time set to: ");
        Serial.println(data_transfer_time);
        // Update LittleFS file
        writeFile(LittleFS, data_transfer_time_Path, data_transfer_time.c_str());
      }
      if (p->name() == PARAM_INPUT_13) {
        saving_time = p->value().c_str();
        Serial.print("saving_time set to: ");
        Serial.println(saving_time);
        // Update LittleFS file
        writeFile(LittleFS, saving_time_Path, saving_time.c_str());
      }
      if (p->name() == PARAM_INPUT_14) {
        displaying_time = p->value().c_str();
        Serial.print("displaying_time set to: ");
        Serial.println(displaying_time);
        // Update LittleFS file
        writeFile(LittleFS, displaying_time_Path, displaying_time.c_str());
      }
      if (p->name() == PARAM_INPUT_15) {
        irr_cal = p->value().c_str();
        Serial.print("irrafiation calibration factor set to: ");
        Serial.println(irr_cal);
        // Update LittleFS file
        writeFile(LittleFS, irr_cal_Path, irr_cal.c_str());
      }
      if (p->name() == PARAM_INPUT_16) {
        pid_set_point = p->value().c_str();
        Serial.print("pid_set_point set to: ");
        Serial.println(pid_set_point);
        // Update LittleFS file
        writeFile(LittleFS, pid_set_point_Path, pid_set_point.c_str());
      }
      if (p->name() == PARAM_INPUT_17) {
        sample_surface = p->value().c_str();
        Serial.print("sample_surface set to: ");
        Serial.println(sample_surface);
        // Update LittleFS file
        writeFile(LittleFS, sample_surface_Path, sample_surface.c_str());
      }
    }
  }
  restart = true;
  request->send(200, "text/plain", "Done. ESP will restart.");
  Serial.println("RestartArduino");
}

void handleStatus(AsyncWebServerRequest * request) {
  // Send response based on the current state of isPCool
  request->send_P(200, "text/plain", isPCool ? "true" : "false");
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
