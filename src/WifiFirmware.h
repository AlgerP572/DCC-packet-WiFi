



#ifndef WifiFirmware_h
#define WifiFirmware_h

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

class WifiFirmware
{   
private:
    // Already defined in elegant OTA.
    // AsyncElegantOTA _asyncElegantOTA;

public:    
    static void begin(AsyncWebServer* server);
};

#endif