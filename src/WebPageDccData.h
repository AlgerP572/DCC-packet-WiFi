#ifndef WebPageDccData_h
#define WebPageDccData_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>
#include <version.h>

class WebPageDccData
{

private: 
    static char _html[];
    static AsyncWebSocket _ws;
    static AsyncEventSource _events;
    static int _millisRollOver;
    static unsigned long _lastMillis;    

    // Timer variables
    static unsigned long _lastTime;
    static unsigned long _lastTimeDCC; 
    static unsigned long _timerDelay;    
    
    static void notifyClients();
    static void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                 AwsEventType type, void *arg, uint8_t *data, size_t len);
    static String processor(const String& var);
    static void GetStats(String& jsonData);    

public:
    static void begin(AsyncWebServer* server);
    static bool loop();
};

#endif // WebPageDccData_h