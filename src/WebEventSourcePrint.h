
#ifndef WebEventSourcePrint_h
#define WebEventSourcePrint_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

#include "Print.h"

class WebEventSourcePrint : public Print
{
    public:
        WebEventSourcePrint() : Print()
        {        
        } 

        // Write a character.
        size_t write(uint8_t c) override
        {
            if(_events == nullptr)
                return 0;

            char msg[2] = {c, 0};

            _events->send(msg,
                "WebEventLog",
                 millis());
            return 1;
        }

        // Write an array of characters.
        size_t write(const uint8_t *buffer, size_t size) override
        {   
            if(_events == nullptr)
                return 0;                

            String logLines;
            logLines.reserve(1280);

            FormatLogAsJson(logLines, reinterpret_cast<const char*>(buffer));
            _events->send(logLines.c_str(),
                "WebEventLog",
                 millis());
            return size;
        } 

        void begin(AsyncEventSource* events)
        {
            _events = events;
        }
    
    private:
        void FormatLogAsJson(String& jsonData, const char* logLines)
        {
            JsonDocument doc;

            doc["Cmd"] = "LogText";
            JsonObject Data = doc["Lines"].to<JsonObject>();
            Data["lines"] = logLines;

            serializeJson(doc, jsonData);
        }

        AsyncEventSource* _events = NULL;
};

#endif // WebEventSourcePrint_h