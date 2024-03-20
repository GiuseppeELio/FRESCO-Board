/* PCool Includes */
#include <PID_v1_rc.h>
#include <arduino-timer.h>

/* Tdrop Includes */
//#include <arduino-timer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MLX90614.h>
#include <RTClib.h>
#include <U8glib.h>
#include <SD.h>
#include <string.h>

//#define __ECHO_ENABLED__
//#define __DEBUG_ENABLED__
//#define __SPLOT_ENABLED__
//#define __TELEPLOT_ENABLED__
//#define __CURRENT_DEBUG_ENABLED__
#define __STARTUP_MESSAGE_ENABLED__

#define __SEND_CURRENT__
#define __SEND_POWER_DENSITY__



/* Firmware Version */
#define __FW_MAJVERSION__               1
#define __FW_MINVERSION__               0
//#define __FW_BUILDVERSION__     0       // if not equal to 0 (zero) EEPROM is reset to default values
#define __FW_AUTHOR__ "RConcas"

#define PCOOL_GET_TEMPERATURE_TIMER     10000     // 10 ms
#define PCOOL_PIDOUT                    1000000   // 1 s

#define PCOOL_GET_CURRENT_TIMER         10000     // 10 ms
#define PCOOL_AVERAGE_CURRENT_TIMER     1000000   // 1 s
#define delayms                         1
#define PID_SAMPLE_TIME                 100        // 200ms

#define ANALOG_CHANNELS                 4

/* Analog FrontEnd parameters */
#define R_SHUNT             1
#define R_LOAD              100
#define PSUPPLY_VOLTAGE     12
#define R_AMPLI             39
#define SAMPLE_SURFACE      0.0036


/* Analog pins */
#define PIN_NTC0        A15
#define PIN_CURR0       A14
#define PIN_NTC1        A13
#define PIN_CURR1       A12
#define PIN_NTC2        A11
#define PIN_CURR2       A10
#define PIN_NTC3        A9
#define PIN_CURR3       A8

#define PIN_PWM0        2
#define PIN_PWM1        3
#define PIN_PWM2        4
#define PIN_PWM3        5


/* PID */
#define PID_PARAMETER_GET_CHAR      "?"
#define PID_PARAMETER_GET_VALUE     -2



/*DEFINITION*/
#define DHTPIN          13
#define DHTPIN2         14
#define DHTPIN3         15
#define ONE_WIRE_BUS2   8 /* Sensor on board*/ /*Remember to not put it in the parallel line of the sensors that go in the external box*/
#define DHTTYPE         DHT22                        // DHT 22  (AM2302)
#define esp8266         Serial1                      //RX and TX settled on Serial1 --> RX2 and TX2 of Arduino Mega
/*NTC parameters*/
#define RT0             10000     // Ω
#define B               3380      //  K NTC part number NXFT15XH103FA2B100 or 3455
#define VCC             5         //Operating Voltage
#define R               10000     //R=10KΩ Pull Up resistor
#define T0              298.15
/*Task times */
//#define RESET_TIME 14400000 //time for reset function every 4 hours in milliseconds -- 7200000 (2 hours)
//unsigned long ResetTime = RESET_TIME;
//#define TASK1 3000                  //gettemp
#define TASK1 2000                  //gettemp
unsigned long TASK2 = 10000;         //Log data and send them using string
unsigned long TASK3 = 3000;
static uint8_t displayState = 0;

/* Pid command structure */
typedef struct _pidCommand{
  String parameter;
  uint8_t channel;
  float value;
} pidCommand;

/* DDS command structure */
typedef struct _pidParameters {
  double PidSetpoint;
  double PidInput;   // ok
  double PidOutput;  // ok
  double Kp;         // ok
  double Ki;         // ok
  double Kd;         // ok
} pidParameters;

  /* PID */
double PidSetpoint = 0;
double PidInput[ANALOG_CHANNELS] = { 0, 0, 0, 0 };
double PidOutput[ANALOG_CHANNELS] = { 0, 0, 0, 0 };
double Kp[ANALOG_CHANNELS] = { 200, 0, 0, 200 };      // ok
double Ki[ANALOG_CHANNELS] = { 0, 0, 0, 0 };  // ok
double Kd[ANALOG_CHANNELS] = { 0, 0, 0, 0 };        // ok

pidParameters heaterControllerParameters[ANALOG_CHANNELS];

PID heaterController[ANALOG_CHANNELS] = {
  PID(&PidInput[0], &PidOutput[0], &PidSetpoint, Kp[0], Ki[0], Kd[0], DIRECT),
  PID(&PidInput[1], &PidOutput[1], &PidSetpoint, Kp[1], Ki[1], Kd[1], DIRECT),
  PID(&PidInput[2], &PidOutput[2], &PidSetpoint, Kp[2], Ki[2], Kd[2], DIRECT),
  PID(&PidInput[3], &PidOutput[3], &PidSetpoint, Kp[3], Ki[3], Kd[3], DIRECT)
  };


/*---------*/
double currentCalibration = 1000 * PSUPPLY_VOLTAGE * R_SHUNT / (R_SHUNT + R_LOAD);
//double POWER_DENSITY_FACTOR = 1000 * (float)PSUPPLY_VOLTAGE * (R_LOAD / (R_SHUNT + R_LOAD)) / SAMPLE_SURFACE;
int count = 0;
long temperatureCounter = 0;
long currentCounter = 0;

double currentValue[ANALOG_CHANNELS] = { 0, 0, 0, 0 };
double currentCalibrationValue[ANALOG_CHANNELS] = { 0, 0, 0, 0 };
double powerDensity[ANALOG_CHANNELS] = { 0, 0, 0, 0 };
double temperatureValue[ANALOG_CHANNELS] = { 0, 0, 0, 0 };
double averageTemperatureValue[ANALOG_CHANNELS] = {0, 0, 0, 0};
double heaterValue[ANALOG_CHANNELS] = { 0, 0, 0, 0 };

const uint8_t currentSensingPin[ANALOG_CHANNELS] = {PIN_CURR0, PIN_CURR1, PIN_CURR2, PIN_CURR3};
const uint8_t temperatureSensingPin[ANALOG_CHANNELS] = {PIN_NTC0, PIN_NTC1, PIN_NTC2, PIN_NTC3};
const uint8_t heaterPin[ANALOG_CHANNELS] = {PIN_PWM0, PIN_PWM1, PIN_PWM2, PIN_PWM3};


Timer<4, micros> timerPcool;  // create a timer with 1 task and microsecond resolution

String receivedString = "";   // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete


/* Function prototypes */
bool GetTemperature(void *);
bool GetCurrent(void *);
bool SendResults(void *);





/*Timer definition*/
Timer<4> timer;                                                   // create a timer with N tasks and microsecond resolution
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0);  // I2C / TWI


/* Values Initialiazation */
// float tempBoard = 0;  //The sensor used to measure the temp. on Board
// float t, h, t2, h2, t3, h3, irr, IR_temp_amb, IR_temp_sky = 0;
// NTC variables initialization
// float RT, RT2, VR, VR2, ln, ln2, T1, T2, T0, VRT, VRT2;
// float RT3, RT4, VR3, VR4, ln3, ln4, T3, T4, VRT3, VRT4;
// float RTbox, VRbox, lnbox, Tbox, VRTbox;

float tempBoard = 0;  //The sensor used to measure the temp. on Board
float t, h, t2, h2, t3, h3, irr, IR_temp_amb, IR_temp_sky = 0;

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




unsigned long startTime = 0;
unsigned long interval = 0;
const unsigned long displayInterval = 2000;  // Adjust the interval as needed

const int numChannels = 5;
const int numOtherSensors = 10;
int analogPins[numChannels] = {A0, A1, A2, A3, A4};
float VRT[numChannels];
float VR[numChannels];
float RT[numChannels];
float ln[numChannels];
float T[numChannels];


/*--- SETUP ---*/
void setup() {
  receivedString.reserve(100);    // reserve 100 bytes for the receivedString
  Serial.begin(115200);  // initialize serial communication at 57600 bits per second:
  esp8266.begin(115200);
  /**/
  //T0 = 25 + 273.15;
  /**/
  Sensors_initialization();
  SD_initialization();
  RTC_initialization();
  delay(20000);                   // aggiunto per garantire che il modulo esp sia pronto alla comunicazione quando necessario
  Wifi_status();
  init_screen();
  /* PID Setup */
  heaterController[0].SetSampleTime(PID_SAMPLE_TIME);
  heaterController[3].SetSampleTime(PID_SAMPLE_TIME);
  heaterController[0].SetMode(AUTOMATIC);  //turn the PID on
  heaterController[3].SetMode(AUTOMATIC);  //turn the PID on
  //PidSetpoint = 22.0;
  /* Current Setup */
  CurrentCalibration();
  analogReference(EXTERNAL);
  /* Timer setup */
  timerPcool.every(PCOOL_GET_TEMPERATURE_TIMER, SetTemperature);
  timerPcool.every(PCOOL_GET_CURRENT_TIMER, GetCurrent);
  timerPcool.every(PCOOL_PIDOUT, SendTemperature);
  timerPcool.every(PCOOL_AVERAGE_CURRENT_TIMER, SendCurrent);

/**/
  timer.every(TASK1, datalog);
  timer.every(TASK2, SendDataToESP8266);
  timer.every(TASK3, displaying_data);

  //timer.every(ResetTime, resetFunc);
  auto active_tasks = timer.size();
  Serial.print("Active task:");
  Serial.println(active_tasks);

  
#ifdef __STARTUP_MESSAGE_ENABLED__
  Serial.print("PCool");
  Serial.print(" v");
  Serial.print(__FW_MAJVERSION__);
  Serial.print(".");
  Serial.print(__FW_MINVERSION__);
  Serial.print("-");
  Serial.println(__FW_AUTHOR__);
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.println(__TIME__);
#endif
}

/*--- LOOP ---*/
void loop() {
  pidCommand command;
  if (stringComplete) {
    ParseCommand(receivedString, &command);
    ExecuteCommand(&command);

    receivedString = "";                    // clear the string
    stringComplete = false;
  }

  //for (int channel = 0; channel < ANALOG_CHANNELS; channel++)
  if(heaterController[0].Compute())   analogWrite(heaterPin[0], PidOutput[0]);
  if(heaterController[3].Compute())   analogWrite(heaterPin[3], PidOutput[3]);

  timer.tick();
  timerPcool.tick();
}


/* Timer Callback functions */

bool SetTemperature(void *) {
  SetTemperatureChannel(0);
  SetTemperatureChannel(3);
  temperatureCounter++;
  return true;
}

bool SendTemperature(void *) {
  SendTemperatureChannel(0);
  SendTemperatureChannel(3);
  temperatureCounter = 0;
  return true;
}

/* Current */
bool GetCurrent(void *) {
  GetCurrentChannel(0);
  GetCurrentChannel(3);
  currentCounter++;
  return true;
}

bool SendCurrent(void *) {
  SendCurrentChannel(0);
  SendCurrentChannel(3);
  currentCounter = 0;
  return true;
}

/* Temperature */
bool SetTemperatureChannel(unsigned char channel) {
  PidInput[channel] = ReadTemperatureChannel(channel);
  return true;
}

float ReadTemperatureChannel(unsigned char channel) {
  float temperature = analogRead(temperatureSensingPin[channel]);
  temperatureValue[channel] += temperature;
  temperature = 1023 / temperature - 1;
  temperature = 10000 / temperature;

  temperature = temperature / 10000;   // (R/Ro)
  temperature = log(temperature);      // ln(R/Ro)
  temperature /= 3380;                 // 1/B * ln(R/Ro)
  temperature += 1.0 / (25 + 273.15);  // + (1/To)
  temperature = 1.0 / temperature;     // Invert
  temperature -= 273.15;               // convert absolute temp to C
  if( temperature > 80 || temperature < -10)  temperature = -273.15;
  return temperature;
}

bool SendTemperatureChannel(unsigned char channel) {

  float temperature;
  temperatureValue[channel] /= temperatureCounter;
  temperatureValue[channel] = 1023 / temperatureValue[channel] - 1;
  temperatureValue[channel] = 10000 / temperatureValue[channel];

  temperature = temperatureValue[channel] / 10000;  // (R/Ro)
  temperature = log(temperature);                   // ln(R/Ro)
  temperature /= 3380;                              // 1/B * ln(R/Ro)
  temperature += 1.0 / (25 + 273.15);               // + (1/To)
  temperature = 1.0 / temperature;                  // Invert
  temperature -= 273.15;                            // convert absolute temp to C

  averageTemperatureValue[channel] = temperature;


#ifdef __SPLOT_ENABLED__
  Serial.print("Ch");
  Serial.print(channel);
  Serial.print("--->Setpoint[°C]:");
  Serial.print(PidSetpoint, 1);
  Serial.print(",Temperature[°C]:");
  Serial.print(temperature);
  Serial.print(",PIDout:");
  Serial.println(PidOutput[channel]);
#endif

#ifdef __TELEPLOT_ENABLED__
  //Serial.print(channel);
  Serial.print(">Setpoint(");
  Serial.print(channel);
  Serial.print(")[°C]:");
  Serial.println(PidSetpoint, 1);
  Serial.print(">Temperature(");
  Serial.print(channel);
  Serial.print(")[°C]:");
  Serial.println(temperature);
  Serial.print(">PIDout(");
  Serial.print(channel);
  Serial.print("):");
  Serial.println(PidOutput[channel]);
#endif

  temperatureValue[channel] = 0;
  return true;
}
/*---*/


/* Current */
bool GetCurrentChannel(unsigned char channel) {
  currentValue[channel] += (long)analogRead(currentSensingPin[channel]);
  return true;
}

bool SendCurrentChannel(unsigned char channel) {

  currentValue[channel] /= currentCounter;
  currentValue[channel] *= currentCalibration;
  currentValue[channel] /= currentCalibrationValue[channel];

  powerDensity[channel] = currentValue[channel]/1000;
  powerDensity[channel] *= PSUPPLY_VOLTAGE;
  powerDensity[channel] /= SAMPLE_SURFACE;

  #ifdef __TELEPLOT_ENABLED__
    #ifdef __SEND_CURRENT__
      Serial.print(">Current(");
      Serial.print(channel);
      Serial.print(")[mA]:");
      Serial.println(currentValue[channel], 1);
    #endif
    #ifdef __SEND_POWER_DENSITY__
      Serial.print(">PowerDensity(");
      Serial.print(channel);
      Serial.print(")[W/mq]:");
      Serial.println(powerDensity[channel], 2);
    #endif
  #endif
  currentValue[channel] = 0;
  return true;
}


bool CurrentCalibration(void) {

  for(char channel = 0; channel<ANALOG_CHANNELS; channel++)
    analogWrite(heaterPin[channel], 255);

  delay(500);  // Settling TIME

  for(char channel = 0; channel<ANALOG_CHANNELS; channel++){
    currentCalibrationValue[channel] = analogRead(currentSensingPin[channel]);

#ifdef __DEBUG_ENABLED__
    Serial.print("Calibration[");
    Serial.print(channel);  
    Serial.print("]:");
    Serial.println(currentCalibrationValue[channel]);
#endif
  }

  for(char channel = 0; channel<ANALOG_CHANNELS; channel++)
    analogWrite(heaterPin[channel], 0);

  return true;
}


/* Serial */
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    receivedString += inChar;
    if (inChar == '\n') stringComplete = true;
  }
}

bool ParseCommand(String inputString, pidCommand* dataStream) {
  /*
  <command><ch>":"<value>
  */

  String inputStream = "";
  String command = "";
  String value ="";
  char cmdIndex = 0;

  inputString.trim();                                               // remove head/end spaces, carrier return,.. any whitespace characters
  cmdIndex = inputString.indexOf(":");                              // separator command to value
  dataStream->parameter = inputString.substring(0, 2);              // put parameter in pidCommand struct
  dataStream->channel = inputString.substring(2, 3).toInt();        // put channel in pidCommand struct

  inputString = inputString.substring(cmdIndex+1);                  // remove command and ': from input string
  if(inputString == PID_PARAMETER_GET_CHAR)
    dataStream->value = PID_PARAMETER_GET_VALUE;
  else 
    dataStream->value = inputString.toFloat();                      // put value in pidCommand struct
  
  return true;
}

bool ExecuteCommand (pidCommand* command){
  if(command->parameter == ""     ||        //struct is in range
      command->channel  == 0xFF   ||
      command->value < PID_PARAMETER_GET_VALUE)
      return false;

  Serial.println("OK");
  if(command->value == PID_PARAMETER_GET_VALUE){    // Get value
    if(!(command->parameter).compareTo("kp")){    
      #ifdef __TELEPLOT_ENABLED__
        Serial.print(">kp");
        Serial.print(command->channel);
        Serial.print(":");
        //Serial.println(Kp[command->channel]);   
        Serial.println(heaterController[command->channel].GetKp());
      #endif
      Serial.print("kp");
      Serial.print(command->channel);
      Serial.print(":");
      Serial.println(heaterController[command->channel].GetKp());      
    }
    if(!(command->parameter).compareTo("ki")){    
      #ifdef __TELEPLOT_ENABLED__
        Serial.print(">ki");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetKi());

        Serial.print(">Ti");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetTi());
      #endif

      Serial.print("ki");
      Serial.print(command->channel);
      Serial.print(":");
      Serial.print(heaterController[command->channel].GetKi());

      Serial.print(",Ti");
      Serial.print(command->channel);
      Serial.print(":");
      Serial.println(heaterController[command->channel].GetTi());
    }
    if(!(command->parameter).compareTo("kd")){     
      #ifdef __TELEPLOT_ENABLED__
        Serial.print(">kd");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetKd());      
        
        Serial.print(">Td");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetTd());
      #endif

      Serial.print("kd");
      Serial.print(command->channel);
      Serial.print(":");
      Serial.print(heaterController[command->channel].GetKd());      

      Serial.print(",Td");
      Serial.print(command->channel);
      Serial.print(":");
      Serial.println(heaterController[command->channel].GetTd());
    }
    if(!(command->parameter).compareTo("sp")){     
      #ifdef __TELEPLOT_ENABLED__
        Serial.print(">sp");
        Serial.print(":");
        Serial.println(PidSetpoint); 
      #endif
      
      Serial.print("sp");    
      Serial.print(":");
      Serial.println(PidSetpoint); 
    }    
  }else{
    if(!(command->parameter).compareTo("kp")){                                 // if command to be executed is swoff
      Kp[command->channel] = command->value;
      heaterController[command->channel].SetTunings(Kp[command->channel], Ki[command->channel], Kd[command->channel]);
    }
    if(!(command->parameter).compareTo("ki")){                                  // if command to be executed is swon
      Ki[command->channel] = command->value;  
      heaterController[command->channel].SetTunings(Kp[command->channel], Ki[command->channel], Kd[command->channel]);
    }
    if(!(command->parameter).compareTo("kd")){                                  // if command to be executed is swon
      Kd[command->channel] = command->value;  
      heaterController[command->channel].SetTunings(Kp[command->channel], Ki[command->channel], Kd[command->channel]);
    }
    if(!(command->parameter).compareTo("sp")){                                  // if command to be executed is swon
      PidSetpoint = command->value;      
    }    

#ifdef __TELEPLOT_ENABLED__
        Serial.print(">kp");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetKp());
      
        Serial.print(">ki");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetKi());

        Serial.print(">Ti");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetTi());
      
        Serial.print(">kd");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetKd());      
        
        Serial.print(">Td");
        Serial.print(command->channel);
        Serial.print(":");
        Serial.println(heaterController[command->channel].GetTd());
      
        Serial.print(">sp");
        Serial.print(":");
        Serial.println(PidSetpoint); 
      #endif

    //heaterController[3].Initialize();
  }  
  return true;
}


/**/


void Sensors_initialization() {
  Wire.begin();
  lightMeter.begin();
  mlx.begin();
  dht.begin();
  sensorsBoard.begin();
  //sensorsBoard.setResolution(RESOLUTION_DALLAS);
}


bool SendDataToESP8266 (void*){
  loggingTemperature();
  GenerateESPString();
  #ifndef __DEBUG_ENABLED__
    PidSetpoint = t3;
  #endif
  esp8266.println(str);
  return true;
}

bool loggingTemperature(void) {
  /**/
  h = dht.readHumidity();
  t = dht.readTemperature();  // Read temperature as Celsius
  if (isnan(h) || isnan(t)) {
    h = 0;
    t = -273.15;
  }
  h2 = dht2.readHumidity();
  t2 = dht2.readTemperature();  // Read temperature as Celsius
  if (isnan(h2) || isnan(t2)) {
    h2 = 0;
    t2 = -273.15;
  }
  h3 = dht3.readHumidity();
  t3 = dht3.readTemperature();  // Read temperature as Celsius
  if (isnan(h3) || isnan(t3)) {
    h3 = 0;
    t3 = -273.15;
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
}

bool GenerateESPString(void){
   /* String Writing the String for the ESP8266*/
    str = String(t) + String(",") + String(h) + String(",") + String(t2) + String(",") +
        String(h2) + String(",") + String(t3) + String(",") + String(h3) + String(",") + 
        String(T[0]) + String(",") + String(T[1]) + String(",") + String(T[2]) + String(",") +
        String(T[3]) + String(",") + String(T[4]) + String(",") + String(tempBoard) +
        String(",") + String(irr) + String(",") + String(IR_temp_amb) + String(",") +
        String(IR_temp_sky);
  
  // /* Power Density */
  str += String(",");
  str += String(powerDensity[0]);
  str += String(",");
  str += String(averageTemperatureValue[0]);
  str += String(",");
  str += String(powerDensity[3]);
  str += String(",");
  str += String(averageTemperatureValue[3]);
  str += String(",");
  str += String(PidSetpoint);
  return true;
}

void loggingNTC() {
  for (uint8_t i = 0; i < numChannels; ++i) {
    VRT[i] = (5.00 / 1023.00) * analogRead(analogPins[i]);  // Acquisition analog value of VRT
    VR[i] = VCC - VRT[i];
    RT[i] = VRT[i] / (VR[i] / R);  // Resistance of RT
    ln[i] = log(RT[i] / RT0);
    T[i] = (1 / ((ln[i] / B) + (1 / T0)));  // Temperature from thermistor
    T[i] -= 273.15;  // Conversion to Celsius
  }
}

void RTC_initialization() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    //while (1);
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
    myFile.print("Date & Time, TA1, H1, TA2, H2, TA3, H3, TS1, TS2, TS3, TS4, Tbx, Tbr, Ir, TAIR, TSIR");
    myFile.println(", T0, PD0, T3, PD3, Tset1, Tset2");
    Serial.println("Date & Time, Tamb 1, H 1, Tamb2, H2, Tamb 3, H3, TS1, TS2, TS3, TS4, Tbox, Tboard, Irr, Tamb IR, TSky IR, T0, PD0, T3, PD3, TSet1, TSet2");
    
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


/* Icons.ino */

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

const unsigned char heat_icon [] PROGMEM = {
	0x06, 0x04, 0x04, 0x00, 0x0e, 0x0c, 0x0c, 0x00, 0x1c, 0x1c, 0x1c, 0x00, 0x18, 0x18, 0x18, 0x00, 
	0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x0c, 0x1c, 0x1c, 0x00, 
	0x0e, 0x0c, 0x0c, 0x00, 0x06, 0x06, 0x06, 0x00, 0x03, 0x07, 0x07, 0x00, 0x03, 0x83, 0x03, 0x00, 
	0x01, 0x81, 0x83, 0x00, 0x01, 0x81, 0x81, 0x80, 0x01, 0x81, 0x83, 0x80, 0x03, 0x83, 0x03, 0x00, 
	0x07, 0x07, 0x07, 0x00, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfe, 
	0x7f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xc0, 
	0x7f, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x60, 0x3f, 0xff, 0xff, 0xe0, 0x7f, 0xff, 0xff, 0xe0, 
	0xe0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff
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
 void Draw_powerDensity() {
  u8g.firstPage();
  do {
    draw();
    u8g.setFont(u8g_font_8x13);
    u8g.setPrintPos(5, 20);
    u8g.print("PD 0");
    u8g.setPrintPos(10, 30);
    u8g.print(powerDensity[0]);
    u8g.setPrintPos(55, 30);
    u8g.print("W/mq");
    u8g.setPrintPos(5, 40);
    u8g.print("PD 3");
    u8g.setPrintPos(10, 50);
    u8g.print(powerDensity[3]);
    u8g.setPrintPos(55, 50);
    u8g.print("W/mq");
    u8g.drawBitmapP(90, 25, 4, 32, heat_icon);
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

bool displaying_data(void) {
  //static uint8_t displayState = 0;
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
      case 9:
      Draw_powerDensity();
      break;
    default:
      // Handle unknown state
      break;
      //return true;
  }

  // Check if it's time to switch to the next state
  if (millis() - startTime >= displayInterval) {
    startTime = millis();  // Reset the start time
    displayState++;

    // Reset state to the initial state if we've reached the end
    if (displayState >= 10) {
      displayState = 0;
    }
  }
  //Serial.print("State: ");
  //Serial.println(displayState);
  return true;
}

/* data_print_log.ino*/
/*Saving data on SD card file and print data on the terminal*/
bool datalog(void *) {
  DateTime now = rtc.now();  //HAVE TO BE SOSTITUTED WITH THE NTP TIME
  Date = String(now.day(), DEC) + String("/") + String(now.month(), DEC) + String("/") + String(now.year(), DEC);
  Time = String(now.hour(), DEC) + String(":") + String(now.minute(), DEC) + String(":") + String(now.second(), DEC);
  Date_Time = String(Date) + String(" ") + String(Time);
  loggingTemperature();
  GenerateESPString();
  str0 = Date_Time + "," + str;
  
  myFile = SD.open("DATA.txt", FILE_WRITE);
  if (myFile)   
    myFile.println(str0);
  else          
    Serial.println("Couldn't print on file");
  myFile.close();
  Serial.println(str0);
  return true;
}


/*void getstring(){
  loggingTemperature();
  GenerateESPString();
  esp8266.println(str); //the string containing the data is sent to the ESP for the web and dashboard
}*/
