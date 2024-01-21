/*Saving data on SD card file and print data on the terminal*/
void datalog() {
  DateTime now = rtc.now();  //HAVE TO BE SOSTITUTED WITH THE NTP TIME
  Date = String(now.day(), DEC) + String("/") + String(now.month(), DEC) + String("/") + String(now.year(), DEC);
  Time = String(now.hour(), DEC) + String(":") + String(now.minute(), DEC) + String(":") + String(now.second(), DEC);
  Date_Time = String(Date) + String(" ") + String(Time);
  loggingTemperature();

  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile) {
    str0 = String(Date_Time) + String(",") + String(str);
    myFile.println(str0);
  }
  myFile.close();
  Serial.println(str0);
}

void getstring() {
  esp8266.println(str);  //the string containing the data is sent to the ESP for the web and dashboard
}