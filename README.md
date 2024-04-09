#       PRC Measurement Station including Tdrop and PCool
![alt text](Fresco_Logo_3.png)
## Introduction and scope 

The code reported in this repository are related to a data logger realization able to measure the temperature drop and the cooling power of "passive radiative cooling" samples.

![alt text](new_boxe_v35.png)

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
