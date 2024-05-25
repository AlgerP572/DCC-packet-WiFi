
#include "WifiSerialDebug.h"
#include "NTPTimeClient.h"

LogLevel Log::_logLevel = // default log levels
    LogLevel::SERIALONOFF |
    // LogLevel::WIFIONOFF |
    LogLevel::INFO |
    //LogLevel::DEBUG |
    LogLevel::ERROR;

std::map<String, SerialDebugCommands>  Log::_debugCommands;
std::map<int, String>  Log::_logLevelNames;

char logBuffer[1024];
WebEventSourcePrint Log::_eventsSerial;
BufferedLogWriter Log::_logWriter(logBuffer, sizeof(logBuffer));

unsigned long Log::_lastMillis = 0;
TaskHandle_t Log::logTask;
SemaphoreHandle_t Log::xMultiPrintMutex;

SerialDebugCommands Log::recvDebugDataMsg(const char* data, size_t len)
{   
    String d(data, len);
    SerialDebugCommands result = SerialDebugCommands::cmdNone;
    Log::println(data, LogLevel::DEBUG);    
   
    auto it = _debugCommands.find(d);
    if(it == _debugCommands.end())
    {
        Log::print("Unknown cmd = ", LogLevel::DEBUG);
        Log::println(d, LogLevel::DEBUG);
        return result;
    }

    result =it->second;
    switch(result)
    {
        case cmdSerialOn:
           _logLevel = _logLevel | LogLevel::SERIALONOFF;
           _logWriter.SetSerialOn();
        break;
        case cmdSerialOff:
           _logLevel = _logLevel & ~LogLevel::SERIALONOFF;
           _logWriter.SetSerialOff();
        break;
        case cmdWifiOn:
            _logLevel = _logLevel | LogLevel::WIFIONOFF;
            _logWriter.SetWifiOn();
        break;
        case cmdWifiOff:
            _logLevel = _logLevel & ~LogLevel::WIFIONOFF;
            _logWriter.SetWifiOff();
        break;
        case cmdInfoOn:
            _logLevel = _logLevel | LogLevel::INFO;
        break;
        case cmdInfoOff:
            _logLevel = _logLevel & ~LogLevel::INFO;
        break;
        case cmdErrorOn:
            _logLevel = _logLevel | LogLevel::ERROR;
        break;
        case cmdErrorOff:
            _logLevel = _logLevel & ~LogLevel::ERROR;
        break;
        case cmdDebugOn:
            _logLevel = _logLevel | LogLevel::DEBUG;
        break;
        case cmdDebugOff:
            _logLevel = _logLevel & ~LogLevel::DEBUG;
        break;
         case cmdLoopOn:
            _logLevel = _logLevel | LogLevel::LOOP;
        break;
        case cmdLoopOff:
            _logLevel = _logLevel & ~LogLevel::LOOP;
        break;
        case cmdWatchdogOn:
            _logLevel = _logLevel | LogLevel::WATCHDOG;
        break;
        case cmdWatchdogOff:
            _logLevel = _logLevel & ~LogLevel::WATCHDOG;
        break;
        case cmdTimeStampsOn:
            _logLevel = _logLevel | LogLevel::TIMESTAMPS;
        break;
        case cmdTImeStampsOff:
             _logLevel = _logLevel & ~LogLevel::TIMESTAMPS;
        break;
        case cmdPrintLevelOn:
            _logLevel = _logLevel | LogLevel::PRINTLEVEL;
        break;
        case cmdPrintLevelOff:
             _logLevel = _logLevel & ~LogLevel::PRINTLEVEL;
        break;
        default:

            // Should never get here.
            result = SerialDebugCommands::cmdNone;
            Log::print("Unknown cmd = ", LogLevel::DEBUG);
            Log::println(d, LogLevel::DEBUG);
        break;
    }
    return result;
}

void  Log::InitializeCommands()
{
    _debugCommands = {
        {"serial:on", cmdSerialOn},
        {"serial:off", cmdSerialOff},
        {"wifi:on", cmdWifiOn},
        {"wifi:off", cmdWifiOff},
        {"info:on", cmdInfoOn},
        {"info:off", cmdInfoOff},
        {"debug:on", cmdDebugOn},
        {"debug:off", cmdDebugOff},
        {"error:on", cmdErrorOn},
        {"error:off", cmdErrorOff},
        {"loop:on", cmdLoopOn},
        {"loop:off", cmdLoopOff},
        {"watchdog:on", cmdWatchdogOn},
        {"watchdog:off", cmdWatchdogOff},
        {"timestamps:on", cmdTimeStampsOn},
        {"timestamps:off", cmdTImeStampsOff},
        {"printlevel:on", cmdPrintLevelOn},
        {"printlevel:off", cmdPrintLevelOff}
    };   

   _logLevelNames = {
        {0x00, " NONE: "},
        {0x01, " SERIALONOFF: "},
        {0x02, " WIFIONOFF: "},
        {0x04, " INFO: "},
        {0x08, " DEBUG: "},
        {0x10, " ERROR: "},
        {0x20, " LOOP: "},
        {0x40, " WATCHDOG: "},
        {0x80, " TIMESTAMPS: "},
        {0x100, " PRINTLEVEL: "}
    };
}

std::vector<String> Log::GetCommands()
 {
    std::vector<String> result;

    for (const auto& [cmd, _] : _debugCommands)
    {
        result.push_back(cmd);
    }

    return result;
}

void Log::begin(AsyncEventSource* events)
{
    if (events == nullptr)
    {
        // Handle the error appropriately       
        Log::println("Error: Events instance is null", LogLevel::ERROR);
        return;
    }

    InitializeCommands();

    _eventsSerial.begin(events);
    _logWriter.begin(&Serial, &_eventsSerial);
    xMultiPrintMutex = xSemaphoreCreateRecursiveMutex();
}

void Log::setup()
{
    xTaskCreate(task,
        "LogTask",
        16000,      // Stack size in bytes. 
        nullptr,        
        1,          // Priority of the task. 
        &logTask);
}

bool Log::loop()
{
    return false;
}

void Log::task(void* data)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while(true)
    {

        // If there has not been log data printed for a refresh time flush
        // the current contents so it can be seen or if the log is full
        // flush right away.        
        _logWriter.WaitBufferFull(pdMS_TO_TICKS(750));
        _logWriter.FlushLog();
    }
}

void Log::print(String& m, LogLevel level)
{   
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY); 
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void Log::print(byte& m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY); 
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void Log::print(bool m, LogLevel level)
{
   if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY); 
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::print(const char* m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY); 
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void Log::print(char* m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY); 
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void Log::print(int m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::print(uint8_t m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::print(uint16_t m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::print(uint32_t m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::print(double m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::print(float m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.print(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}


// Print with New Line

void  Log::println(String& m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void Log::println(byte& m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void Log::println(bool m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(const char *m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(char *m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(int m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);    
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(uint8_t m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(uint16_t m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(uint32_t m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(float m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void  Log::println(double m, LogLevel level)
{
    if(IsLevelOn(level) == false)
        return;

    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
    PrintTags(level);
    _logWriter.println(m);
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

void Log::TakeMultiPrintSection()
{
    xSemaphoreTakeRecursive(xMultiPrintMutex, portMAX_DELAY);
}

void Log::GiveMultiPrintSection()
{
    xSemaphoreGiveRecursive(xMultiPrintMutex);
}

bool Log::IsSerialOn()
{
    return (_logLevel & LogLevel::SERIALONOFF) == LogLevel::SERIALONOFF;
}

bool Log::IsWifiOn()
{
    return (_logLevel & LogLevel::WIFIONOFF) == LogLevel::WIFIONOFF;
}

bool Log::IsLevelOn(LogLevel level)
{
    return (_logLevel & level) == level;
}

void Log::PrintTags(LogLevel level)
{
    if(_logWriter.GetNewLine() == false)
        return;

    if(IsLevelOn(LogLevel::TIMESTAMPS))
    {
        String formattedTime;
        TimeClient::getFormattedDate(formattedTime);

         _logWriter.print(formattedTime);
    }
    if(IsLevelOn(LogLevel::PRINTLEVEL))
    {        
        _logWriter.print(_logLevelNames[(int) level]);
    }

    _logWriter.SetNewLineFalse();
}

    
    

