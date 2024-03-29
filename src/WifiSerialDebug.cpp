
#include "WifiSerialDebug.h"

LogLevel Log::_logLevel = // default log levels
    LogLevel::SERIALONOFF |
    // LogLevel::WIFIONOFF |
    LogLevel::INFO |
    //LogLevel::DEBUG |
    LogLevel::ERROR;

std::map<String, SerialDebugCommands>  Log::_debugCommands;

void Log::recvDebugDataMsg(uint8_t *data, size_t len)
{   
    String d = "";
    for(int i=0; i < len; i++){
        d += char(data[i]);
    }   

    switch(_debugCommands[d])
    {
        case cmdSerialOn:
           _logLevel = _logLevel | LogLevel::SERIALONOFF;
        break;
        case cmdSerialOff:
           _logLevel = _logLevel = _logLevel & ~LogLevel::SERIALONOFF;
        break;
        case cmdWifiOn:
            _logLevel = _logLevel | LogLevel::WIFIONOFF;
        break;
        case cmdWifiOff:
            _logLevel = _logLevel & ~LogLevel::WIFIONOFF;
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
        default:
        break;
    }
}

void  Log::InitializeCommands()
{
    _debugCommands["serial:on"] = cmdSerialOn;
    _debugCommands["serial:off"] = cmdSerialOff; 
    _debugCommands["wifi:on"] = cmdWifiOn;
    _debugCommands["wifi:off"] = cmdWifiOff; 
    _debugCommands["info:on"] = cmdInfoOn;
    _debugCommands["info:off"] = cmdInfoOff; 
    _debugCommands["debug:on"] = cmdDebugOn;
    _debugCommands["debug:off"] = cmdDebugOff; 
    _debugCommands["error:on"] = cmdErrorOn;
    _debugCommands["error:off"] = cmdErrorOff; 
    _debugCommands["loop:on"] = cmdLoopOn;
    _debugCommands["loop:off"] = cmdLoopOff; 
    _debugCommands["watchdog:on"] = cmdWatchdogOn;
    _debugCommands["watchdog:off"] = cmdWatchdogOff;   
}

void Log::begin(AsyncWebServer* server)
{
    InitializeCommands(); 
    
    WebSerial.begin(server);
    WebSerial.msgCallback(recvDebugDataMsg);    
}

// Print
void Log::print(String& m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void  Log::print(const char* m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void Log::print(char* m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void Log::print(int m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void  Log::print(uint8_t m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void  Log::print(uint16_t m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void  Log::print(uint32_t m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void  Log::print(double m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}

void  Log::print(float m, LogLevel level)
{
    printWifi(m, level);
    printSerialPort(m, level);
}


// Print with New Line

void  Log::println(String& m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);   
}

void  Log::println(const char *m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

void  Log::println(char *m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

void  Log::println(int m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

void  Log::println(uint8_t m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

void  Log::println(uint16_t m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

void  Log::println(uint32_t m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

void  Log::println(float m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

void  Log::println(double m, LogLevel level)
{
    printlnWifi(m, level); 
    printlnSerialPort(m, level);
}

// Seial Port print

void Log::printSerialPort(String& s, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(s);
}

void Log::printSerialPort(const char *m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;     

    Serial.print(m);
}

void Log::printSerialPort(char *m, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(m);
}

void Log::printSerialPort(int m, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(m);
}

void Log::printSerialPort(uint8_t m, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(m);
}

void Log::printSerialPort(uint16_t m, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(m);
}

void Log::printSerialPort(uint32_t m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(m);
}

void Log::printSerialPort(double m, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(m);
}

void Log::printSerialPort(float m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.print(m);
}

// Seial Port println 

void Log::printlnSerialPort(String& s, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(s);
}

void Log::printlnSerialPort(const char *m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

void Log::printlnSerialPort(char *m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

void Log::printlnSerialPort(int m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

void Log::printlnSerialPort(uint8_t m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

void Log::printlnSerialPort(uint16_t m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

void Log::printlnSerialPort(uint32_t m, LogLevel level)
{
    if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

void Log::printlnSerialPort(float m, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

void Log::printlnSerialPort(double m, LogLevel level)
{
   if(IsSerialOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    Serial.println(m);
}

// Wifi print

void Log::printWifi(String& s, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(s);
}

void Log::printWifi(const char *m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

void Log::printWifi(char *m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

void Log::printWifi(int m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

void Log::printWifi(uint8_t m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

void Log::printWifi(uint16_t m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

void Log::printWifi(uint32_t m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

void Log::printWifi(double m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

void Log::printWifi(float m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.print(m);
}

// wifi println

void Log::printlnWifi(String& s, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(s);
}

void Log::printlnWifi(const char *m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
}

void Log::printlnWifi(char *m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
}

void Log::printlnWifi(int m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
}

void Log::printlnWifi(uint8_t m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
}

void Log::printlnWifi(uint16_t m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
}

void Log::printlnWifi(uint32_t m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
}

void Log::printlnWifi(float m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
}

void Log::printlnWifi(double m, LogLevel level)
{
   if(IsWifiOn() == false)
        return;
    if(IsLevelOn(level) == false)        
        return;

    WebSerial.println(m);
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

    
    

