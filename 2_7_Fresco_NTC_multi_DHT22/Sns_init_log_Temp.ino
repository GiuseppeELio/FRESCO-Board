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

