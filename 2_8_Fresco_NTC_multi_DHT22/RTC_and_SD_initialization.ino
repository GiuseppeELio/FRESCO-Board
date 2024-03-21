void RTC_initialization() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  rtc.adjust(DateTime((__DATE__), (__TIME__))); // Use this command in order to set the rigth Date and time on the RTC
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
  DateTime now = rtc.now(); // Assuming you have an RTC named "rtc" initialized

  // Create folder structure for year, month, and day
  String folderPath = "/" + String(now.year()) + "/" + formatTwoDigits(now.month()) + "/" + formatTwoDigits(now.day()) + "/";
  SD.mkdir(folderPath); // Create folders if they don't exist

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
