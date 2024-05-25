#include "WifiFirmware.h"
#include "WifiSerialDebug.h"

void WifiFirmware::begin(AsyncWebServer* server)
{
    if (server == nullptr)
    {
        // Handle the error appropriately       
        Log::println("Error: Server instance is null", LogLevel::ERROR);
        return;
    }

    // Access is via the /update route.
    AsyncElegantOTA.begin(server);    // Start ElegantOTA
    Log::println("OTA firmware web page started ok", LogLevel::DEBUG);
}