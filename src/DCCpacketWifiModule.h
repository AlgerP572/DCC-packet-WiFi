#ifndef DCCpacketWifiModule_h
#define DCCpacketWifiModule_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <driver/temp_sensor.h>

class DCCpacketWifiModule
{

public:
    static void setup();
    static void begin();
    static void loop();

    static float ReadCoreTemp();
};

#endif