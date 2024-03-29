
#ifndef WifiConnection_h
#define WifiConnection_h

#include <Arduino.h>
#include <ESPAsyncWiFiManager.h>

class WifiConnection
{

public:
    static bool setup(AsyncWebServer* server, DNSServer* dns, bool reset);
};

#endif