#ifndef DCCPacketDecoderModule_h
#define DCCPacketDecoderModule_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <driver/temp_sensor.h>
#include "DCCStatistics.h"
#include "StringBuilder.h"

class DCCPacketDecoderModule
{

public:
    static void setup();
    static void begin();
    static bool loop();
    static void task(void* data);

    static Statistics GetLastKnwonStats();
    static void GetDCCPacketStats(String& jsonData, Statistics& lastKnownStats);
    static void GetDCCPacketBytes(String& jsonData);
    static unsigned int GetRefreshDelay();
    static void SetRefreshDelay(unsigned int delay);
    static bool processCommands(const char* command, int length);

private:
    static Statistics _lastKnownStats;

    static void clearHashList();
    static bool processDCC(StringBuilder &output);
    static void DecodePacket(StringBuilder &sbTemp, int inputPacket, bool isDifferentPacket);
    static void printPacketBits(Print &output, int index);
    static void printPacketIntervals(Print &output, int index);
};

#endif