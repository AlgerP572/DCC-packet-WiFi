#include "DCCPacketData.h"

DCCPacketData DCCPackets::dccPackets[MAX_PACKETS];


void DCCPackets::SetPacketString(int inputPacket, const char* decodedPacket)
{
    if(inputPacket >= MAX_PACKETS)
        return;

    strncpy(dccPackets[inputPacket].byteString,
        decodedPacket,
        MAX_BYTESDATA);
}

void DCCPackets::SetIntervals(int inputPacket, volatile unsigned int* intervals)
{
    if(inputPacket >= MAX_PACKETS)
        return;

    for(int i = 0; i < intervals[0]; i++)
    {
        dccPackets[inputPacket].bitIntervals[i] = intervals[i];
    }    
}


int DCCPackets::GetIntervalLength(int inputPacket)
{
    return dccPackets[inputPacket].bitIntervals[0];
}

unsigned int* DCCPackets::GetInterval(int inputPacket)
{
    return dccPackets[inputPacket].bitIntervals;
}

void DCCPackets::Reset()
{
    for(int i = 0; i < MAX_PACKETS; i++)
    {
        // reset string to empty.
        dccPackets[i].byteString[0] = 0;
    }
}

char* DCCPackets::GetPacketSrting(int inputPacket)
{
    return dccPackets[inputPacket].byteString;
}
