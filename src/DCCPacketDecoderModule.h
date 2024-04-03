#ifndef DCCPacketDecoderModule_h
#define DCCPacketDecoderModule_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_log.h>
#include <driver/temp_sensor.h>
#include "DCCStatistics.h"

class DCCPacketDecoderModule
{

public:
    static void setup();
    static void begin();
    static void loop();

    static Statistics GetLastKnwonStats();
    static void GetDCCPacketStats(String& jsonData, Statistics& lastKnownStats);
    static void GetDCCPacketBytes(String& jsonData, Statistics& stats);
    static unsigned int GetRefreshDelay();
    static void SetRefreshDelay(unsigned int delay);

private:
    static Statistics _lastKnownStats;

    static void clearHashList();
    static bool processDCC(Print &output);
    static void DecodePacket(Print &output, int inputPacket, bool isDifferentPacket);
    static bool processCommands();
    static void printPacketBits(Print &output, int index);
    static void printPacketIntervals(Print &output, int index);
};

#endif