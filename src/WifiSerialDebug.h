#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include <map>

#include "BufferedLogWriter.h"
#include "WebEventSourcePrint.h"

#define ENUM_FLAG_OPERATOR(T,X) inline T operator X (T lhs, T rhs) { return (T) (static_cast<int>(lhs) X static_cast<int>(rhs)); }
#define ENUM_FLAGS(T) \
enum class T; \
inline T operator ~ (T t) { return (T) (~static_cast<int>(t)); } \
ENUM_FLAG_OPERATOR(T,|) \
ENUM_FLAG_OPERATOR(T,^) \
ENUM_FLAG_OPERATOR(T,&) \
enum class T

typedef  enum serialDebugCommands
{
    cmdNone,
    cmdCustom,
    cmdSerialOn,
    cmdSerialOff,
    cmdWifiOn,
    cmdWifiOff,
    cmdInfoOn,
    cmdInfoOff,
    cmdDebugOn,
    cmdDebugOff,
    cmdErrorOn,
    cmdErrorOff,
    cmdLoopOn,
    cmdLoopOff,
    cmdWatchdogOn,
    cmdWatchdogOff,
    cmdTimeStampsOn,
    cmdTImeStampsOff,
    cmdPrintLevelOn,
    cmdPrintLevelOff
} SerialDebugCommands;

ENUM_FLAGS(LogLevel)
{
    NONE = 0x00,
    SERIALONOFF = 0x01,
    WIFIONOFF = 0x02,
    INFO = 0x04,
    DEBUG = 0x08,
    ERROR = 0x10,
    LOOP = 0x20,
    WATCHDOG = 0x40,
    TIMESTAMPS = 0x80,
    PRINTLEVEL = 0x100
};

class Log
{

private:    
    
    static LogLevel _logLevel;
    static std::map<String, SerialDebugCommands> _debugCommands;
    static std::map<int, String> _logLevelNames;
    static BufferedLogWriter _logWriter;
    static WebEventSourcePrint _eventsSerial;
    static unsigned long _lastMillis;

    static TaskHandle_t logTask;
    static SemaphoreHandle_t xMultiPrintMutex;

    static bool IsSerialOn();
    static bool IsWifiOn();
    static bool IsLevelOn(LogLevel level);
    static void PrintTags(LogLevel level);

    static void InitializeCommands();
    
public:    
    static void print(String& m, LogLevel level);
    static void print(byte& m, LogLevel level);
    static void print(bool m, LogLevel level);
    static void print(const char* m, LogLevel level);
    static void print(char* m, LogLevel level);
    static void print(int m, LogLevel level);
    static void print(uint8_t m, LogLevel level);
    static void print(uint16_t m, LogLevel level);
    static void print(uint32_t m, LogLevel level);
    static void print(double m, LogLevel level);
    static void print(float m, LogLevel level);

    // Print with New Line
    static void println(String& m, LogLevel level);    
    static void println(byte& m, LogLevel level);
    static void println(bool m, LogLevel level);
    static void println(const char* m, LogLevel level);
    static void println(char* m, LogLevel level);
    static void println(int m, LogLevel level);
    static void println(uint8_t m, LogLevel level);
    static void println(uint16_t m, LogLevel level);
    static void println(uint32_t m, LogLevel level);
    static void println(float m, LogLevel level);
    static void println(double m, LogLevel level);

    static void TakeMultiPrintSection();
    static void GiveMultiPrintSection();

    static void begin(AsyncEventSource* events);
    static void setup();
    static bool loop();
    static void task(void* data);

    static SerialDebugCommands recvDebugDataMsg(const char *data, size_t len); 
    static std::vector<String> GetCommands();
};

#endif