#include <Wire.h>

#include "DCCpacketWifiModule.h"
#include "WifiSerialDebug.h"
#include "ConfigLoader.h"

static const char *LOG_TAG = "ESP32";

void DCCpacketWifiModule::setup()
{ 
    JsonDocument* jsonDataObj = ConfigLoader::getDocPtr(String("/configdata/phcfg.cfg"));
    if (jsonDataObj != NULL)
    {        
        delete(jsonDataObj); 


         Serial.println("Wifi Packet View loaded"); 
    }
    else
    {
        Serial.println("/configdata/phcfg.cfg not Found."); 
    }
}

void DCCpacketWifiModule::begin()
{
}

void DCCpacketWifiModule::loop()
{     
}

float DCCpacketWifiModule::ReadCoreTemp()
{
    float result;
    temp_sensor_read_celsius(&result);
    return result;
}