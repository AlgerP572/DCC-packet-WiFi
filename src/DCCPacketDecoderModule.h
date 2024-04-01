#ifndef DCCPacketDecoderModule_h
#define DCCPacketDecoderModule_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <driver/temp_sensor.h>

class DCCPacketDecoderModule
{

public:
    static void setup();
    static void begin();
    static void loop();

    static void GetDCCPacketStats(String& jsonData);
    static void GetDCCPacketBytes(String& jsonData);
    static unsigned int GetRefreshDelay();

private:
    static void clearHashList();
    static bool processDCC(Print &output);
    static void DecodePacket(Print &output, int inputPacket, bool isDifferentPacket);
    static bool processCommands();
    static void printPacketBits(Print &output, int index);
    static void printPacketIntervals(Print &output, int index);
};

#endif