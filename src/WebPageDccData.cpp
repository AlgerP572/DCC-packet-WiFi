#include "NTPTimeClient.h"
#include "WebPageDccData.h"
#include "WifiSerialDebug.h"

#include "DCCpacketWifiModule.h"
#include "DCCPacketDecoderModule.h"
#include "ConfigLoader.h"
#include "CpuUsage.h"


char WebPageDccData::_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>WiFi DCC Packet Viewer</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <link rel="stylesheet" type="text/css" href="style.css">
  <script
      src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.9.1/chart.min.js"
      integrity="sha512-ElRFoEQdI5Ht6kZvyzXhYG9NqjtkmlkfYk0wr6wHxU9JEHakS7UJZNeml5ALk+8IKlU6jDgMabC3vkumRokgJA=="
      crossorigin="anonymous"
      referrerpolicy="no-referrer"
    ></script>    
  <style>
    .topnav { background-color: #000080; }   
    .reading { font-size: 1.4rem; }
  </style>
</head>
<body>
  <div class="topnav">
    <div class="tool-bar">        
        <a href="/">
            <button class="btn-group">DCC Data</button>
        </a>
        <a href="/update">
            <button>FW Update</button>
        </a>        
        <a href="/wifilog">
            <button>Wifi Log</button>
        </a>
        <h1>DCC Packet Display</h1>     
    </div>    
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">  
        <div class="button-container">
          <button id="btnCapture" class="button" onClick="capture(this)">Capture</button>
          <button id="btnContinuous" class="button" onClick=" continuous(this)">Continuous</button>
          <button id="btnStop" class="button" onClick=" stop(this)">Stop</button>
        </div>                            
        <p><table class="datatable">
        <tr>
          <th>DCC Statistics</th>                   
        </tr>
         <tr>          
          <td><span id="bitCount">%DCCbitcount%</span></td>                
        </tr>
        <tr>
          <td><span id="packetStats">%DCCpacketStats%</span></td>        
        </tr>
         <tr>         
          <td><span id="zeroBits">%zeroBits%</span></td>                 
        </tr>
         <tr>        
          <td><span id="oneBits">%oneBits%</span></td>        
        </tr>
        <tr>        
          <td><span id="cpu">--</span></td>        
        </tr>
        <tr>
          <td><span id="dccPacketOne">%dccPacketOne%</span></td>     
        </tr>
        <tr>
          <td><span id="dccPacketTwo">%dccPacketTwo%</span></td>     
        </tr>
        <tr>
          <td><span id="dccPacketThree">%dccPacketThree%</span></td>     
        </tr>
        <tr>
          <td><span id="dccPacketFour">%dccPacketFour%</span></td>     
        </tr>
        <tr>
          <td><span id="dccPacketFive">%dccPacketFive%</span></td>     
        </tr>
        <tr>
          <td><span id="dccPacketSix">%dccPacketSix%</span></td>     
        </tr>
        <tr>
          <td><span id="dccPacketSeven">%dccPacketSeven%</span></td>     
        </tr>
        <tr>
          <td><span id="dccPacketEight">%dccPacketEight%</span></td>     
        </tr>
      </table></p>
      </div>      
      <div class="card">
        <div class="tool-bar-graph">
          <button id="buttonCsv" class="profilebutton">To Csv</button>
          <p class="card-title">DCC Data</p>
        </div>
          <canvas id="chart-speed-data" width="600" height="400"></canvas>
        </div>
    </div>
  </div>
  <div class="topnav">
    <p><table class="footertable">
      <tr>
        <td>Date / Time</td>
        <td><span id="datetime">%datetime%</span></td>
        <td>IP Address</td>
        <td><span id="ipaddress">%ipaddress%</span></td>
        <td>Firmware Version</td>
        <td><span id="fwversion">%fwversion%</span></td>    
      </tr>
      <tr>
        <td>System Uptime / Cpu Load</td>
        <td><span id="systemuptime">%systemuptime%</span></td>
        <td>Signal Strength</th>
        <td><span id="signalstrength">%signalstrength%</span></td>
        <td>Available RAM/Flash</td>
        <td><span id="ramflash">%ramflash%</span></td>    
      </tr>
      <tr>
        <td>Core Temp</td>
        <td><span id="coretemp">%coretemp%</span></td>
        <td>Access Point</td>
        <td><span id="accesspoint">%accesspoint%</span></td>
        <td>Available SPIFFS</td>
        <td><span id="spiffs">%spiffs%</span></td>    
      </tr>      
    </table></p>
  </div>
    <script src="canvasjsascsv.min.js"></script>
    <script src="papaparse.min.js"></script> 
    <script src="DccDataDisplay.js"></script>  
</body>
</html>)rawliteral";

AsyncWebSocket WebPageDccData::_ws("/wsdccdata");
AsyncEventSource WebPageDccData::_events("/eventsdccdata");
unsigned long WebPageDccData::_lastTime = 0;
unsigned long WebPageDccData::_lastTimeDCC = 0;
unsigned long WebPageDccData::_timerDelay = 1000;
int WebPageDccData::_millisRollOver = 0;
unsigned long WebPageDccData::_lastMillis = 0;

void WebPageDccData::handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    Log::println("WebSocket event", LogLevel::INFO);

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
       
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, (char *)data);

        if (error)
        {
            Log::println("Error deserializing json data.", LogLevel::ERROR);
            return;
        }

        if (doc.containsKey("Cmd") == false)
        {
            Log::println("Cmd not provided nothing to do.", LogLevel::ERROR);
            return;
        }

        String thisCmd = doc["Cmd"];
        Log::println(thisCmd, LogLevel::INFO);

        
        if (thisCmd == "SetSensor")
        {
            String subCmd = doc["SubCmd"];
            Log::println(subCmd, LogLevel::INFO);

            if (subCmd == "Capture")
            {
                DCCPacketDecoderModule::SetRefreshDelay(1);
            }   
            if (subCmd == "Continuous")
            {                
                // Setting refresh to default for DCC Inspector.
                DCCPacketDecoderModule::SetRefreshDelay(4);
            }       
            if (subCmd == "Stop")
            {                
                // Setting refresh delay to once per day effectively
                // stopping the loop refresh.
                DCCPacketDecoderModule::SetRefreshDelay(86400);
            }
        }
        if (thisCmd == "CfgData") // Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg", "FileName":"name"}
        {
            String cmdType = doc["Type"];
            String fileName = doc["FileName"];
            
            Log::TakeMultiPrintSection();
            Log::print("Request for config file: ", LogLevel::INFO);
            Log::println(fileName, LogLevel::INFO);
            Log::GiveMultiPrintSection();

            String cmdMsg;
            cmdMsg.reserve(512);
            cmdMsg += "{\"Cmd\":\"CfgData\", \"ResetData\":true, \"Type\":\"";
            cmdMsg += cmdType; 
            cmdMsg += "\",\"Data\":";
            
            String configData;
            ConfigLoader::readFile("/configdata/" + fileName, configData);            
            cmdMsg += configData;
            cmdMsg += "}";
           
            _events.send(cmdMsg.c_str(),
                        "CfgData",
                         millis());
        }
        if (thisCmd == "STATS")
        {           
            String jsonData;
            jsonData.reserve(640);

            GetStats(jsonData);
            
            Log::println(jsonData, LogLevel::DEBUG);
            _events.send(jsonData.c_str(),
                         "STATS",
                         millis());
        }
    }
}

void WebPageDccData::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                                    void *arg, uint8_t *data, size_t len)
{ 
    switch (type)
    {
    case WS_EVT_CONNECT: 
        Log::TakeMultiPrintSection();        
        Log::print("WebSocket client #", LogLevel::INFO);
        Log::print(client->id(), LogLevel::INFO);
        Log::print(" connected from ", LogLevel::INFO);
        Log::println(client->remoteIP().toString().c_str(), LogLevel::INFO);        
        Log::GiveMultiPrintSection();
        break;
    case WS_EVT_DISCONNECT:
        Log::TakeMultiPrintSection();
        Log::print("WebSocket client # ", LogLevel::INFO);
        Log::print(client->id(), LogLevel::INFO);
        Log::println(" disconnected", LogLevel::INFO);        
        Log::GiveMultiPrintSection();
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
        Log::TakeMultiPrintSection();   
        Log::print("WS Pong Id: ", LogLevel::DEBUG);
        Log::println(client->id(), LogLevel::DEBUG);
        Log::GiveMultiPrintSection();
        break;
    case WS_EVT_ERROR:
        Log::TakeMultiPrintSection();  
        Log::print("WS Error Id: ", LogLevel::ERROR);
        Log::println(client->id(), LogLevel::ERROR);
        Log::GiveMultiPrintSection();
        break;
    }
}

void WebPageDccData::begin(AsyncWebServer *server)
{
    if (server == nullptr)
    {
        // Handle the error appropriately       
        Log::println("Error: Server instance is null", LogLevel::ERROR);
        return;
    }

    _ws.onEvent(onEvent);
    server->addHandler(&_ws);

    // Handle Web Server Events
    _events.onConnect([](AsyncEventSourceClient *client)
    {
        if(client->lastId())
        {
            Log::TakeMultiPrintSection();
            Log::print("Client reconnected! Last message ID that it got is: ", LogLevel::INFO);
            Log::println(client->lastId(), LogLevel::INFO);
            Log::GiveMultiPrintSection();
        }

        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second   
        client->send("hello!", "", millis(), 1000);
    });

    server->addHandler(&_events);

    // Route for root / web page
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200,
            "text/html",
            _html,
            processor);
    });   
}

String WebPageDccData::processor(const String& var)
{
    return String();
}

bool WebPageDccData::loop()
{
    if (millis() < _lastMillis)
        _millisRollOver++; // update ms rollover counter
    else
        _lastMillis = millis();

    if (_ws.count() == 0)
    {
        return false;
    }
    _ws.cleanupClients();    

    if ((millis() - _lastTimeDCC) > (DCCPacketDecoderModule::GetRefreshDelay() * 1000))
    {

        if(DCCPacketDecoderModule::GetRefreshDelay() <= 1)
        {
            // THis is for capture mode. After the one capture is
            // complete turn off.
            DCCPacketDecoderModule::SetRefreshDelay(86400);
        }


        // Get speed data
        String latestDCCData;
        latestDCCData.reserve(1024);

        // Ensure DCCPacketDecoderModule::loop is called before this loop in main.
        // DCCPacketDecoderModule::loop will capture the latest stats for us.  This
        // was done to preserve the original code as much as possible. It also helps
        // to maintain coherency between loops so they report the same stats.
        Statistics stats = DCCPacketDecoderModule::GetLastKnwonStats();
        
        DCCPacketDecoderModule::GetDCCPacketStats(latestDCCData, stats);
        if (latestDCCData.isEmpty() == false)
        {
            Log::println(latestDCCData.c_str(), LogLevel::LOOP);
            _events.send(latestDCCData.c_str(),
                         "DccStats",
                         millis());
        }

        String latestDCCPackets;
        latestDCCPackets.reserve(1024);

        DCCPacketDecoderModule::GetDCCPacketBytes(latestDCCPackets);
        if (latestDCCPackets.isEmpty() == false)
        {
            Log::println(latestDCCPackets.c_str(), LogLevel::LOOP);
            _events.send(latestDCCPackets.c_str(),
                         "PacketBytes",
                         millis());
        }

        _lastTimeDCC = millis();
        return true;
    }

    if ((millis() - _lastTime) > _timerDelay)
    {
        Log::println("-----", LogLevel::LOOP);

        // Send Events to the Web Client with the Sensor Readings
        _events.send("ping", "", millis());

        String statsData;
        statsData.reserve(640);
        GetStats(statsData);
            
        Log::println(statsData, LogLevel::LOOP);
        _events.send(statsData.c_str(),
            "STATS",
            millis());

        _lastTime = millis();
    }
    return false;
}

void WebPageDccData::GetStats(String& jsonData)
{
    JsonDocument doc;

    doc["Cmd"] = "STATS";
    JsonObject data = doc["Data"].to<JsonObject>();
    float float1 = (_millisRollOver * 4294967296) + millis(); // calculate millis including rollovers
    data["uptime"] = round(float1);

   
    String formattedTime;
    TimeClient::getFormattedDate(formattedTime);
    data["systime"] = formattedTime;
    data["cpuload"] = CpuUsage::GetCpuLoad();    
    data["freemem"] = String(ESP.getFreeHeap());
    data["freeflash"] = String(ESP.getFlashChipSize());
    data["totaldisk"] = String(SPIFFS.totalBytes());
    data["useddisk"] = String(SPIFFS.usedBytes());
    data["freedisk"] = String(SPIFFS.totalBytes() - SPIFFS.usedBytes());
    data["version"] = VERSION;
    data["ipaddress"] = WiFi.localIP().toString();
    data["sigstrength"] = WiFi.RSSI();
    data["apname"] = WiFi.SSID();

    data["temp"] = DCCpacketWifiModule::ReadCoreTemp();    

    serializeJson(doc, jsonData);
}