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
      }
    }
    restart = true;
    request->send(200, "text/plain", "Done. ESP will restart, connect to INFLUXDB_URL: " + INFLUXDB_URL + " and INFLUXDB_DB_NAME:" + INFLUXDB_DB_NAME);
  }
