#       PRC Station
![alt text](https://github.com/GiuseppeELio/FRESCO-Board/blob/Tdrop-and-ESP/Pictures/Fresco_Logo_3.png)
## Introduction and scope 

This work present a simple way to build an open source station to measure the themal heating (or cooling), humidity and solar irradicance saving the data in local and sharing them by an IOT Cloud platform over ThingSpeak (through cloud or local server). 

The final station looks like 
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/Real_PRC.jpg?raw=true)

It allows measuring the temperature from 6 sensors applied to 4 different samples (T1, T2, T3 and T4), another one inside the box (Tbox) to measure the temperature above the area covered by samples, another one is used to measure the temperature close to the electronic apparatus (identified as T_board), and another one for the ambient temperature (Tamb) and humidity. The system is also equipped with a sensor to measure the solar irradiance. Techical details about the electronic components used for this goal is reported in the next lines. All this measured value are reported in the following plots
* The first plot reports in the top panel the temperature for Tamb (outside the box), T1, T2, T3, T4 and Tbox (all sensors in the same box, no samples) while the second plot (bottom) displays the humidity and the irradiance.
![alt text](https://github.com/GiuseppeELio/FRESCO-Board/blob/main/Pictures/20_10_22_data.png?raw=true)
* A differnet way to plot the data, the difference between the Tamb and the Tbox, and the Tsample versus Tamb and Tbox, allows to evaluate the consistence about all the sensors placed in the polystyrene box, they reports the same value in their standard deviation (accuracy ± 0.5°C).
![alt text](https://github.com/GiuseppeELio/FRESCO-Board/blob/main/Pictures/20_10_22_Tdiff.png?raw=true)
After that we conducted a couple of measurements across different days on different materials that posseses or not passive radiative cooling features reveling the following behaviours as reported in the next two plots. 
* The plots shows in the top panel the temperature for Tamb, T1, T2, T3, T4 and Tbox while the bottom one displays the humidity and the irradiance.
![alt text](https://github.com/GiuseppeELio/FRESCO-Board/blob/main/Pictures/Date_28-1_10_22_data.png?raw=true)
* These other plots report the difference between the Tamb and Tbox, the T of each samples versus the Tamb or versus the Tbox.
![alt text](https://github.com/GiuseppeELio/FRESCO-Board/blob/main/Pictures/Date_28-1_10_22_Tdiff.png?raw=true)
##       Hardaware Details

The code and the electronic sketch to build a measurement station for Passive Radiative Cooling are shown in the next lines

The system is based on a Arduino development board. In the proposed case an Arduino due has been used.

The manufactured board appear as reported in the following image

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/sketch_scheda.png?raw=true)

In order to realize this Arduino shield with all connection and connector for thermal measurements, the design has been initially designed in Fusion 360, you can use a PCB software that you prefer.

First step: draw the PCB using a two layer board, top layer appears in red and the bottom one is blue. The combination of the two layers is also reported for a cohmprensive view. 

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/PCB_Wiring_sketch.png?raw=true)

Once the PCB dra is ready, the design can be inspected and produced by a virtual process, well known as CAM, thar allows seeing the final shape and the board footprint. Here, the top layer and the bottom one are reported as they will be produced during the PCB manufacturing.

![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/Produced_PCB_2D.png?raw=true)

Finally, the PCB is produced by CAM, using a 3D cad software, the rendering of the final system is reported. It is useful to know the footprint of each component, the required space and how the final system will appear. 

The final rendering, along a top view and two lateral ones of the board is
![alt text](https://github.com/GiuseppeELio/PRC_Station/blob/main/Pictures/Render_Board_annotations_2.png?raw=true)


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
