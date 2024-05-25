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




