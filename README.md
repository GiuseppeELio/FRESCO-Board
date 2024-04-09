#       PRC Measurement Station including Tdrop and PCool
![alt text](Fresco_Logo_3.png)
## Introduction and scope 

The code reported in this repository are related to a data logger realization able to measure the temperature drop and the cooling power of "passive radiative cooling" samples.

![alt text](new_boxe_v35.png)

The board as shown in the figure is composed by a main board, a TDrop shield that allows measuring the temperature for multiple samples, the solar irradiance, the sky temperature using an IR sensor, ambient temperature and relative humidity using a DHT22, store data on a onboard SD card. It is also equipped with a RTC clock for the correct time and a ESP8266-12F module for the WiFi connection and communication. The PCool shield is equipped with NTC to measure the sample temperature and a PID system to warm-up them to the setting point temperature that usually is the ambient temperature. 

##       Software details

In order to use the selected sensors such as: BH1750 (irradiance), Dallas DS18B20 (Temperature), DHT22 (temp and Humidity) and DS3231 (clocker)
it is necessary to install/ include the following libraries 

**BH1750** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/BH1750-1.3.0.zip

**Dallas DS18B20** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DS18B20-master.zip and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/Dallas_temp.zip
with the dallas libraries it is also necessary to install the **One Wire**
https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/OneWire-master.zip

**DHT22** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DHT.zip

**Clocker DS3231** and **RTC lib** https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DS3231%20(1).zip
 and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/RTClib-2.0.3.zip
 and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/Arduino-Due-RTC-Library-master.zip
 
 While for the **SD card reader** (jointed with the clocker allows building a data logger) it is necessary to install: 
 https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/SD-master.zip

 The system is also equipped with a IR termometer **mlx90614** able to measure near and far field temperature using a differential quad IR sensor. It is used to monitor the sky temperature and the cloud level above the setup.

## ESP8266 WiFi connection and data transmissione. 

As reported in the pdf file "FRESCO Board startup_Software" the FRESCO board is equipped with a WiFi network board to communicate and exchange information through a network and show the data directly on your device by a hot spot netwrok or connecting on the local IP of the selected WiFi. On the other hand, it allows sending the data on a remote server using Raspberry or a Linux PC/Server and then save the data on "InfluxDB V2" and display them on "Grafana" dashboard. 

An example of the dashboard displaying the usefull data is reported below

%%% Work in progress%%% The dashboard Json code will be released soon. 
