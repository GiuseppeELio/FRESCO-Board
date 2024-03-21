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