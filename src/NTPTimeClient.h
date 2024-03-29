#ifndef NTPTimeClient_h
#define NTPTimeClient_h

#include <NTPClient.h>
#include <WiFiUdp.h>

class TimeClient
{ 
    static WiFiUDP _ntpUDP;
    static NTPClient _timeClient;

public:
    static void begin();
    static bool update();
    static bool forceUpdate();
    static void getFormattedDate(String& formattedTime);
    static void setTimeOffset(int timeOffset);
    
};

#endif