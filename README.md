# PRC_Station

%%%------HARDWARE DETAILS------%%%

The code and the electronic sketch to build a measurement station for Passive Radiative Cooling


The system is based on a Arduino development board. In the proposed case an Arduino due has been used.

The final rendering board looks like (TOP view) \n
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Scheda_Wifi_Fan_Top.png?raw=true)


While the side view looks like
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Scheda_WiFi_Fan_3.png?raw=true)

The PCB board used as a shield board to avoid annoying wiring connection has the following electronic scheme

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/PCB_footprint_arduino%20v28.pdf?raw=true)

It is based on a 2-layer PCB. Here, the blue lines are the bottom layer and the red one is the top layer. 

%%%------CODE------%%%

In order to use the selected sensors such as: BH1750 (irradiance), Dallas DS18B20 (Temperature), DHT22 (temp and Humidity) and DS3231 (clocker)
it is necessary to install/ include the following libraries 

\textbf{BH1750} https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/BH1750-1.3.0.zip

Dalla DS18B20 https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DS18B20-master.zip and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/Dallas_temp.zip

DHT22 https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DHT.zip

Clocker DS3231 and RTC lib https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/DS3231%20(1).zip
 and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/RTClib-2.0.3.zip
 and https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/Arduino-Due-RTC-Library-master.zip
 
 While for the SD card reader (jointed with the clocker allows building a data logger) it is necessary to install: 
 https://github.com/GiuseppeELio/PRC_Station/blob/main/Libraries/SD-master.zip
 
 
