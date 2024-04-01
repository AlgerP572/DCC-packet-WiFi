#ifndef DCCPacketData_h
#define DCCPacketData_h

#include <Arduino.h>
#include "Config.h"

#define MAX_PACKETS 16
#define MAX_BYTESDATA 128
#define MAX_INTERVALSDATA 256


typedef struct
{
    char byteString[MAX_BYTESDATA];
    unsigned int bitIntervals[MAX_INTERVALSDATA];
} DCCPacketData;

class DCCPackets
{
    private:
        static DCCPacketData dccPackets[];

    public:
        static void SetPacketString(int inputPacket, const char* decodedPacket);
        static void SetIntervals(int inputPacket, volatile unsigned int* intervals);
        static void Reset();
        static char* GetPacketSrting(int inputPacket);        
        static int GetIntervalLength(int inputPacket);
        static unsigned int* GetInterval(int inputPacket);
};


#endif