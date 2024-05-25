#ifndef WebPageWifiLog_h
#define WebPageWifiLog_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>
#include <version.h>

#include "WifiSerialDebug.h"

typedef bool (* SerialCommandProcessor)(const char* comamnd, int length);

class WebPageWifiLog
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
    static void recvDebugDataMsg(char *data, size_t len);
    static bool ProcessAllCommands(const String& command);
    static void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    static void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                 AwsEventType type, void *arg, uint8_t *data, size_t len);
    static String processor(const String& var);
    static void GetStats(String& jsonData);
    static void GetCommands(String& jsonData, std::vector<String> commands);
    static void UpdateLogDisplayOptions(const SerialDebugCommands command, String& updateOptions);

public:
    static AsyncEventSource* GetEvents();    

    static void begin(AsyncWebServer* server);
    static bool loop();

    static void AddCustomCommandProcessor(SerialCommandProcessor commandProcessor);
    static void SendCommandList(const std::vector<String> commands);
};

#endif