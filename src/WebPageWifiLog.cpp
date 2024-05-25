#include "NTPTimeClient.h"
#include "WebPageWifiLog.h"
#include "WifiSerialDebug.h"

#include "DCCpacketWifiModule.h"
#include "ConfigLoader.h"
#include "CpuUsage.h"

char WebPageWifiLog::_html[] PROGMEM = R"rawliteral(
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
  <script src="https://microsoft.github.io/monaco-editor/node_modules/monaco-editor/min/vs/loader.js"></script>   
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
        <h1>WiFi Log Viewer</h1>
    </div>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">  
        <div class="button-container">
            <input type="text" class="combo" list="combo-options" style="width:350px;" placeholder="type a text command and hit send or select from the list" id="command">
            <datalist id="combo-options">
            </datalist>   
            <button id="sendStop" class="button" onClick="send(this)">Send</button>
        </div>                            
        <p><table class="datatable">
        <tr>
          <th>Log Display Options</th>                   
        </tr>
         <tr>          
          <td><span id="destinationoptions">
            <label>Destination(s): </label>
            <input type="checkbox" id="seriallog" name="seriallogCheck" onclick='serialLogClick(this);' checked >
            <label> To Serial Port  </label>
            <input type="checkbox" id="wifilog" name="wifilogCheck" onclick='wifiLogClick(this);' checked >
            <label> To Wifi  </label>
          </span></td>                
        </tr>
        <tr>
          <td><span id="logleveloptions">
            <label>Level(s): </label>
            <input type="checkbox" id="debuglog" name="debuglogCheck" onclick='debugLogClick(this);' checked >
            <label>Debug  </label>
            <input type="checkbox" id="looplog" name="looplogCheck" onclick='loopLogClick(this);' checked >
            <label>Loop  </label>
            <input type="checkbox" id="errorlog" name="errorlogCheck" onclick='errorLogClick(this);' checked >
            <label>Error  </label>
            <input type="checkbox" id="infolog" name="infologCheck" onclick='infoLogClick(this);' checked >
            <label>Info  </label>
          </span></td>
        </tr>
        <tr>
          <td><span id="logtagoptions">
            <label>Tags(s): </label>
            <input type="checkbox" id="timelog" name="timelogCheck" onclick='timeLogClick(this);' checked >
            <label>Time  </label>
            <input type="checkbox" id="levellog" name="levellogCheck" onclick='levelLogClick(this);' checked >
            <label>Level  </label>            
          </span></td>
        </tr>  
      </table></p>
      </div>      
      <div class="card">
        <div class="tool-bar-graph">
            <input type="checkbox" id="autoscroll" name="autoScrollCheck" onclick='autoScrollClick(this);' checked >
            <label> Auto Scroll </label>
            <button id="clear" class="profilebutton" onclick='clearClick(this);'>Clear</button>
            <p class="card-title">  Log </p>
        </div class="tool-bar-graph">           
            <div id="editor" style="width:550px;height:400px;border:1px solid grey"></div>
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
        <td><span id="fwversion">%fwversion%</span></td    
      </tr>
      <tr>
        <td>System Uptime / Cpu Load</td>
        <td><span id="systemuptime">%dsystemuptime%</span></td>
        <td>Signal Strength</td>
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
    <script src="WifiLogDisplay.js"></script>
    <script src="LanguageLog.js"></script>
</body>
</html>)rawliteral";

AsyncWebSocket WebPageWifiLog::_ws("/wswifilog");
AsyncEventSource WebPageWifiLog::_events("/eventswifilog");
unsigned long WebPageWifiLog::_lastTime = 0;
unsigned long WebPageWifiLog::_lastTimeDCC = 0;
unsigned long WebPageWifiLog::_timerDelay = 1000;
int WebPageWifiLog::_millisRollOver = 0;
unsigned long WebPageWifiLog::_lastMillis = 0;

std::vector<SerialCommandProcessor> _customSerialCommands;

void WebPageWifiLog::handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    Log::println("WebSocket event", LogLevel::INFO);

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = '\0';       
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, reinterpret_cast<char*>(data));

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

        String thisCmd = doc["Cmd"].as<String>();
        Log::println(thisCmd, LogLevel::INFO);

        
        if (thisCmd == "LogCmd")
        {
            String subCmd = doc["SubCmd"].as<String>();
            Log::println(subCmd, LogLevel::DEBUG); 
            ProcessAllCommands(subCmd);
        } 
        if (thisCmd == "CfgData") // Config Request Format: {"Cmd":"CfgData", "Type":"pgxxxxCfg", "FileName":"name"}
        {
            String cmdType = doc["Type"].as<String>();
            String fileName = doc["FileName"].as<String>();
            
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
        if(thisCmd == "APPENDCOMMANDS")
        {
            // Add commands to the comboBox of the log view.
            WebPageWifiLog::SendCommandList(Log::GetCommands());
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

void WebPageWifiLog::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
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

void WebPageWifiLog::begin(AsyncWebServer *server)
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
            Log::print("Client reconnected! Last message ID that it got is: ", LogLevel::INFO);
            Log::println(client->lastId(), LogLevel::INFO);        
        }

        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second   
        client->send("hello!", "", millis(), 1000);
    });

    server->addHandler(&_events);

    // Route for root / web page
    server->on("/wifilog", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200,
            "text/html",
            _html,
            processor);
    });   
}

String WebPageWifiLog::processor(const String& var)
{
    return String();
}

void WebPageWifiLog::AddCustomCommandProcessor(SerialCommandProcessor commandProcessor)
{
    _customSerialCommands.push_back(commandProcessor);
}

void WebPageWifiLog::SendCommandList(const std::vector<String> commands)
{

    String commandsData;
    commandsData.reserve(640);
    GetCommands(commandsData, commands);
            
    Log::println(commandsData, LogLevel::DEBUG);
    _events.send(commandsData.c_str(),
        "APPENDCOMMANDS",
        millis());
}

bool WebPageWifiLog::loop()
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

    if (Serial.available())
    {        
        String command = Serial.readStringUntil('\r');              
        ProcessAllCommands(command);
    }
    return true;
}

bool WebPageWifiLog::ProcessAllCommands(const String& command)
{
    SerialDebugCommands result = SerialDebugCommands::cmdNone;    
    result = Log::recvDebugDataMsg(command.c_str(), command.length());

    if(result == SerialDebugCommands::cmdNone)
    {
        for(std::vector<SerialCommandProcessor>::iterator it = _customSerialCommands.begin(); it != _customSerialCommands.end(); ++it)
        {
            bool extraResult = (*it)(command.c_str(), command.length());
            if(extraResult)
                result = SerialDebugCommands:: cmdCustom;
                break;
        }
    }
    else
    {
        String updateOptions;
        updateOptions.reserve(256);
        UpdateLogDisplayOptions(result, updateOptions);

         _events.send(updateOptions.c_str(),
            "DISPLAYUPDATE",
            millis());
    }
    return result;
}

void WebPageWifiLog::UpdateLogDisplayOptions(const SerialDebugCommands command, String& updateOptions)
{
    JsonDocument doc;

    doc["Cmd"] = "DISPLAYUPDATE";
    JsonObject Data = doc["Data"].to<JsonObject>();

    switch(command)
    {
        case SerialDebugCommands::cmdSerialOn:
            Data["Serial"] = true;
        break;
        case SerialDebugCommands::cmdSerialOff:
            Data["Serial"] = false;
        break;
        case SerialDebugCommands::cmdWifiOn:
            Data["Wifi"] = true;
        break;
        case SerialDebugCommands::cmdWifiOff:
            Data["Wifi"] = false;
        break;

        case SerialDebugCommands::cmdDebugOn:
            Data["Debug"] = true;
        break;
        case SerialDebugCommands::cmdDebugOff:
            Data["Debug"] = false;
        break;
        case SerialDebugCommands::cmdLoopOn:
            Data["Loop"] = true;
        break;
        case SerialDebugCommands::cmdLoopOff:
            Data["Loop"] = false;
        break;
        case SerialDebugCommands::cmdErrorOn:
            Data["Error"] = true;
        break;
        case SerialDebugCommands::cmdErrorOff:
            Data["Error"] = false;
        break;
        case SerialDebugCommands::cmdInfoOn:
            Data["Info"] = true;
        break;
        case SerialDebugCommands::cmdInfoOff:
            Data["Info"] = false;
        break;

        case SerialDebugCommands::cmdTimeStampsOn:
            Data["TimeStamps"] = true;
        break;
        case SerialDebugCommands::cmdTImeStampsOff:
            Data["TimeStamps"] = false;
        break;
        case SerialDebugCommands::cmdPrintLevelOn:
            Data["PrintLevel"] = true;
        break;
        case SerialDebugCommands::cmdPrintLevelOff:
            Data["PrintLevel"] = false;
        break;
    }

    serializeJson(doc, updateOptions);
}

void WebPageWifiLog::GetStats(String& jsonData)
{
    JsonDocument doc;

    doc["Cmd"] = "STATS";
    JsonObject Data = doc["Data"].to<JsonObject>();
    float float1 = (_millisRollOver * 4294967296) + millis(); // calculate millis including rollovers
    Data["uptime"] = round(float1);

   
    String formattedTime;
    TimeClient::getFormattedDate(formattedTime);
    Data["systime"] = formattedTime;
    Data["cpuload"] = CpuUsage::GetCpuLoad();
    Data["freemem"] = String(ESP.getFreeHeap());
    Data["freeflash"] = String(ESP.getFlashChipSize());
    Data["totaldisk"] = String(SPIFFS.totalBytes());
    Data["useddisk"] = String(SPIFFS.usedBytes());
    Data["freedisk"] = String(SPIFFS.totalBytes() - SPIFFS.usedBytes());
    Data["version"] = VERSION;
    Data["ipaddress"] = WiFi.localIP().toString();
    Data["sigstrength"] = WiFi.RSSI();
    Data["apname"] = WiFi.SSID();

    Data["temp"] = DCCpacketWifiModule::ReadCoreTemp();    

    serializeJson(doc, jsonData);
}

void WebPageWifiLog::GetCommands(String& jsonData, std::vector<String> commands)
{
    JsonDocument doc;

    doc["Cmd"] = "Commands";
    JsonObject Data = doc["Data"].to<JsonObject>();
    JsonArray jsonCommands = Data["commands"].to<JsonArray>();

    for(int i = 0; i < commands.size(); i++)
    {
        jsonCommands.add(commands[i]);
    }

    serializeJson(doc, jsonData);
}


AsyncEventSource* WebPageWifiLog::GetEvents()
{    
    return &_events;
}