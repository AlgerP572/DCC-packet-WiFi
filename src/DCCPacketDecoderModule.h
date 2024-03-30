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

private:
    static void clearHashList();
    static bool processDCC(Print &output);
    static void DecodePacket(Print &output, int inputPacket, bool isDifferentPacket);
    static bool processCommands();
    static void printPacketBits(Print &output, int index);
};

#endif