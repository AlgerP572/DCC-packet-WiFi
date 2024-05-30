# DCC Inspector- Background
EX‑DCCInspector is a packet sniffing tool that can connect directly to the signal pins on the Command Station or to the track depending on your circuit.  The source code for this utility is open source and can be found at the git hub repository:

https://github.com/DCC-EX/DCCInspector-EX

In addition there is excellent support information at the following site:

https://dcc-ex.com/ex-dccinspector/index.html#gsc.tab=0

# DCC-packet-WiFi
This github repository comprises a derivative work from EX‑DCCInspector. The derivative software supports over the air (OTA) firmware (FW) download and OTA logging.  It also provides a Wifi accessible web page that can be accssed from a web browser to display. Shown below is a sreenshot of the UI.

![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/0801d99c-f9e8-4729-8c23-cde3e3890973)

# Building / Compiling DCC-packet-WiFi
The DCC-packet-WiFi code has been converted to a platform.io project hosted by microsoft visual studio code.
Both the IDE and platformIO are free to use.  The main motivation for this change was to support incremental
builds which lead to build / compile times on average of less than 40 seconds.  The repo has the required
launch file in the .vscode folder in the src tree.

![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/bcd87202-8e13-41dd-aee4-56eed1fb7781)

The required platformio.ini file setup to build for the Arduino-Metro is also provided in the main src folder. This links to all of the external libraries as well as specifies the spiffs settings for the memory layout. The min-spiffs.csv is also provided in the main src folder.  Arduino-Metro was chosen for this project since it has a large flash space and built in WiFi capability.  The code has also been written to take adavantage of both of the Cores on the chip.

```
; PlatformIO Project Configuration File
;
;   Build options: build flags, source filter
;   Upload options: custom upload port, speed and extra flags
;   Library options: dependencies, extra library storages
;   Advanced options: extra scripting
;
; Please visit documentation for the other options and examples
; https://docs.platformio.org/page/projectconf.html

[env]
lib_deps =  https://github.com/me-no-dev/AsyncTCP.git
            https://github.com/alanswx/ESPAsyncWiFiManager.git
            https://github.com/me-no-dev/ESPAsyncWebServer.git
            https://github.com/ayushsharma82/AsyncElegantOTA.git                       
            https://github.com/bblanchon/ArduinoJson.git            
            https://github.com/taranais/NTPClient.git

extra_scripts =  pre:buildscript_versioning.py

[env:adafruit_metro_esp32s3]
platform = espressif32
framework = arduino
board = adafruit_metro_esp32s3
board_build.partitions = default_16MB.csv
monitor_speed = 115200
upload_speed = 115200
build_type = debug
monitor_filters = esp32_exception_decoder
```

# Hardware Interface

Please refer to the DCC-Ex Inspector github page for information for interfacing the Arduino with track level DCC signals.

https://github.com/DCC-EX/DCCInspector-EX

Currently the code in this repository is setup to use the default ESP32 pin i.e. Pin2 for the dcc input.

> [!CAUTION]
> Warning: Do not directly connect the Arduino Metro or other Microcontrollers directly to the track.
> The high track voltage will destroy the microcontroller and possibly your computer!!!

Below is a list of hardware I have used with the Arduino Metro and I know work:

## DCC Interface Breakout Board with Grove Port

Interface board to convert the track signal of a DCC model railroad to Arduino levels (3.3V or 5V)

https://www.tindie.com/products/tanner87661/dcc-interface-breakout-board-with-grove-port/

| | |
|---|---|
| ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/b24a129e-d93a-4bf5-8256-dcf4959b1310) | ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/ad77c2a1-253a-4073-88f4-5576c30c42a7) |


## Tinkerface - Arduino Interface Shield

Arduino Shield providing DCC and Loconet Interface

https://www.tindie.com/products/tanner87661/tinkerface-arduino-interface-shield/

![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/62e01d59-788c-43d9-9c47-0fc7eab4e995)

# First Time WiFi Setup

DCC packet wifi viewer uses the open source library ESP Async Wifi Manager to manage the connection to the wifi network.  After the first download of the FW and spiffs binaries a one time setup is required to configure the Arduino for Wifi access.  For some Arduino devcies, including the arduino Metro, it may be required to hit the reset button in order to start the firware after downloading binaries.  Once the reset button has been hit the FW will run.  Since the FW has never been run before it will enter into Wifi configuration mode which starts an access point that allows for Wifi configuration.  The access point should show up in the Wifi settings on most smartphones as Wireless-DCC Get-Setup.  Click on this access point and this should show the Wifi Setup screen.  Follow the prompts and provide the name of the desired Network and its password as shown below:

| | | |
|---|---|---|
| ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/84861569-11ec-4761-a3cb-a8179dbd8d10) | ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/e5659c4c-66f2-41d5-83d2-94262167de42) | ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/9e5871b4-be42-445f-a8a3-1e309e2ba4c3) |

Once the setup has been save the FW will restart and connect to your Wifi network using the credentials previously saved above.  To see the ip address that was assigned to the device you can look into your wifi router to see attached devices and their ipaddress.  Once the address is known enter it into the Web browser of your choice and the DCC packet wifi UI should be displayed.  If all is going well you should see the time and CPU load changing once per second.

| | |
|---|---|
| ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/1b9e27dd-f598-48ca-83da-3944edd8e069) | ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/aaff2063-3b38-405d-88bc-d80ea484e10a) |

# Over the Air (OTA) FW Update

The firmware only needs to be updated using the serial port the first time it is installed.  Subsequent firware updates can still be applied via the serial port but DCC packet wifi viewer also includes support for the open source library ElegantOTA.  This library allows the firware and spiffs data to be updated using the wireless network.  The main advantage to this approach is that the firmware can be updated in place without be attached to the PC.  It can be updated without having to remove the Arduino device from the layout.  A second advantage is that using the Wifi download is much faster that the serial download. The library is simple to use. To access the OTA functions hit the firware update button on the main button bar then simply click on the type of file to download either firmware of spiffs.  This will pop-up a file selection that allows to select the file. Hit OK and the download will begin.

| | |
|---|---|
| ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/932896e0-0144-4ab3-96e9-8038c9e603c8) | ![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/c70c2f32-16ec-4a20-a22a-e831cc9dd295) |

More information on the elegant OTA library can be found here:

https://github.com/ayushsharma82/ElegantOTA

# DCC Data

The dcc data tab or ui page is the default page that appears when the dcc packet sniffer application first starts.  At any time the user may return to this page but hitting the DCC data button in the top button bar.

![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/4608e433-1801-43d0-badd-6285503bba8d)


The page shows the same statitistics as the original dcc inspector packet sniffer on the left side in a tabular format. Up to eight different dcc packets will be dispalyed per measurement cycle.  Dcc packet wifi viewer also shows the timings for the first dcc packet in each grouping in a graphical format.  Each byte is separated into its own plot and each byte is displayed in a different color.  The plots may be saved to a csv file for further analysis.  The data capture can be paused and resumed in order to get a closer look at dcc packets of interest by using the three buttons above the statitics display.

# Wifi Log

The Wifi Log tab or ui page serves as the command line interface and output for DCC packet Wifi.  This page allows for the configuration of all of the logging options and can be configured for speed or debugging information as required.  The underlying logging subsystem runs using Rtos tasks and therfore is fully supported on both CPU cores of an ESP32 processor. Shown below is an example of the Wifi log tab:

![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/4a1341e2-44ef-4429-b41e-f677b8bbacd7)

The destinations, log levels and tags are all configurable from the UI.  Default settings for the tab can be changed from the config file stored in the spiffs drive. As can be seen the log destination can be either the Serial port Wifi or both.  This allows complete ssupport in both wired and non-wired configurations.  The wireless option can be advantageous for on the layout debugging in real time. Shown below is an example of WiFi and Serial logging using Arduino IDE as the Serial monitor:

![image](https://github.com/AlgerP572/DCC-packet-WiFi/assets/13104848/2fe45581-4f59-419a-9d15-628125f082c7)





















