#include "WifiFirmware.h"

void WifiFirmware::begin(AsyncWebServer* server)
{
    // Access is via the /update route.
    AsyncElegantOTA.begin(server);    // Start ElegantOTA
}