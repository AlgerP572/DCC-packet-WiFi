#include <Arduino.h>
#include <SPIFFS.h>

#include "WebServer.h"
#include "WebPageDccData.h"
#include "WebPageWifiLog.h"
#include "WifiConnection.h"
#include "WifiSerialDebug.h"
#include "WifiFirmware.h"
#include "NTPTimeClient.h"
#include "CpuUsage.h"

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
    if(SPIFFS.begin(true) == false)
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
    Log::begin(WebPageWifiLog::GetEvents());
  
    // Start supported services
    WebPageDccData::begin(server);
    WebPageWifiLog::begin(server);
    WifiFirmware::begin(server);
    WebServer::begin();

    CpuUsage::setup();

    // Start underlying hardware modules
    DCCpacketWifiModule::setup();
    DCCPacketDecoderModule::setup();
    WebPageWifiLog::AddCustomCommandProcessor(DCCPacketDecoderModule::processCommands);
         
    // Metro ESP32-S3 no screen.
//    M5.Lcd.println("HTTP server started"); 

    // The Metro ESP32-S3 has no screen.
     Serial.println("HTTP server started");

    // After this point use Serial logging functions as
    // spareingly as possible.  The Log class should handle
    // as much as the serial traffic as possible.  This
    // will avoid race conditions since it it Multi-task safe.
    Log::setup();    
}

void loop()
{ 
    u32_t currentTime = millis();  

    if(!TimeClient::update())
    {
        TimeClient::forceUpdate();
    }
    
    if ((currentTime - _lastTime) > _timerDelay)
    {
        Log::TakeMultiPrintSection();
        Log::print("Time: ", LogLevel::WATCHDOG);
        Log::println(currentTime, LogLevel::WATCHDOG);
        Log::GiveMultiPrintSection();

        _lastTime = currentTime;
        
        // The formattedDate comes with the following format:
        // 2018-05-28T16:00:13Z
        // We need to extract date and time
        String formattedTime;
        TimeClient::getFormattedDate(formattedTime);
        Log::println(formattedTime, LogLevel::WATCHDOG);
    }

    // Important DCCPacketDecoderModule must be before any other web pages
    // or modules that access the DCC packet stats.
    // This line captures the current DCC statistics
    // and caches the results. Latest results can
    // be retrieved with DCCPacketDecoderModule::GetLastKnwonStats()   
    if(DCCPacketDecoderModule::loop() == false)
    if(WebPageDccData::loop() == false)
    if(WebPageWifiLog::loop() == false)
    if(Log::loop() == false);
    // The above odd if statement is trying to stagger the tasks
    // so they run semi round robin one at at time...
   
    // Giving the maximum time for other taks to execute.
    // Trying to keep this loop running at 10 Hz to service
    // UI requests web socket events etc.
    u32_t endTime = millis();
    u32_t loopTime = endTime - currentTime;
    Log::print("loopTime: ", LogLevel::LOOP);
    Log::println((int) loopTime, LogLevel::LOOP);

    long loopTaskTime = 100u - loopTime;
    long loopDelay = std::max(loopTaskTime, (long) 100);
   
    delay(loopDelay);   
}
