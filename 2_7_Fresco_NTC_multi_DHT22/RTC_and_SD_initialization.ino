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
  Serial.print(F("Initializing SD card..."));

  if (!SD.begin(chipSelect)) {
    Serial.println(F("initialization failed!"));
    sd_status = "Fail";
    return;
  }
  Serial.println(F("initialization done."));
  sd_status = "Done";
  //open file
  myFile = SD.open("DATA.txt", FILE_WRITE);
  // if the file opened ok, write to it:
  if (myFile) {
    Serial.println("File opened ok");
    // print the headings for our data
    myFile.println("New data acquisition");
    myFile.println("Date & Time, TA1, H1, TA2, H2, TA3, H3, TS1, TS2, TS3, TS4, Tbx, Tbr, Ir, TAIR, TSIR");
    Serial.println("Date & Time, Tamb 1, H 1, Tamb2, H2, Tamb 3, H3, TS1, TS2, TS3, TS4, Tbox, Tboard, Irr, Tamb IR, TSky IR");
  }
  myFile.close();
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
