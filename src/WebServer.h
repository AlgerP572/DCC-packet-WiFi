
#ifndef WebServer_h
#define WebServer_h

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

class WebServer
{

private:
    // Set web server port number to 80
    static AsyncWebServer _server;
    static DNSServer _dns;

public:
    static AsyncWebServer* get();
    static DNSServer* dns();
    static void begin();
};

#endif