#include <Arduino.h>
#include <SPIFFS.h>

#include "WebServer.h"
#include "WebPageTrackMeasuring.h"
#include "WifiConnection.h"
#include "WifiSerialDebug.h"
#include "WifiFirmware.h"
#include "NTPTimeClient.h"

#include "DCCpacketWifiModule.h"
#include "DCCPacketDecoderModule.h"

unsigned long _lastTime;  
unsigned long _timerDelay = 10000;


void setup()
{
    // No screen on ESP32-S3
//    M5.begin(); 
//    M5.Lcd.setTextSize(3);
//    M5.Lcd.setRotation(3);  
//    M5.Lcd.println("DCC-packet-Wifit");

    WiFi.mode(WIFI_STA); 
    Serial.begin(115200);
    Serial.println("DCC-packet-Wifi");

    AsyncWebServer* server = WebServer::get();
    DNSServer* dns = WebServer::dns();

    bool result = WifiConnection::setup(server, dns, false);

    if(result == false) 
    {
     
        Serial.println("Failed to connect");
//        M5.Lcd.println("Failed to connect");
        ESP.restart();
    }
  
    // If you get here you have connected to the WiFi    
    Serial.println("connected...");
//    M5.Lcd.println("Engine: ");    
//    M5.Lcd.println(WiFi.localIP());

    // I have found using the goolge standard nameserver a little more robust.
    // Than the default provided by my ISP.
    // WiFi.config(WiFi.localIP(),
    //      WiFi.gatewayIP(),
    //      WiFi.subnetMask(),
    //      IPAddress(8,8,8,8)); 

    // Initialize a NTPClient to get time
    TimeClient::begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    // GMT +1 = 3600
    // GMT +8 = 28800
    // GMT -1 = -3600
    // GMT 0 = 0
    TimeClient::setTimeOffset(-25200); // southern california.

    // This needs to be before anything that will acess SPIFFS data
    // like config files.
    if(!SPIFFS.begin(true))
    {
        // Chicken and egg here.  Can't use LOG since its not started yet
        // but we might want to put cfg data to control the log so this
        // needs to be first.
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    server->serveStatic("/", SPIFFS, "/");


    // Note: Before this point OTA logging does not work.
    // The next line starts the servcies required for OTA
    // logging. Anything using OTA logging including web
    // pages must be after this line.
    Log::begin(server);
  
    // Start supported services
    WebPageTrackMeasuring::begin(server);
    WifiFirmware::begin(server);
    WebServer::begin();
  

    // Start underlying hardware modules
    DCCpacketWifiModule::setup();
    DCCPacketDecoderModule::setup();
         
    // Metro ESP32-S3 no screen.
//    M5.Lcd.println("HTTP server started"); 

    // The Metro ESP32-S3 has no screen.
     Serial.println("HTTP server started");
}

void loop()
{ 
    u32_t time = millis();    

    if(!TimeClient::update())
    {
        TimeClient::forceUpdate();
    }
    
    if ((time - _lastTime) > _timerDelay)
    {
        Log::print("Time: ", LogLevel::WATCHDOG);
        Log::println(time, LogLevel::WATCHDOG);
        _lastTime = time;
        
        // The formattedDate comes with the following format:
        // 2018-05-28T16:00:13Z
        // We need to extract date and time
        String formattedTime;
        TimeClient::getFormattedDate(formattedTime);
        Log::println(formattedTime, LogLevel::WATCHDOG);
    }

    delay(2);
    WebPageTrackMeasuring::loop();
    delay(2);
    DCCPacketDecoderModule::loop();
   
    // This will "feed the watchdog".
    delay(2);
    return;  
}
