#ifndef Log_h
#define Log_h

#include <Arduino.h>
#include <WebSerial.h>
#include <map>

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
    cmdWatchdogOff
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
    WATCHDOG = 0x40
};

class Log
{

private:    

    // Web Serial already defines this.
    // static WebSerial _webSerial; 
    static LogLevel _logLevel;
    static std::map<String, SerialDebugCommands> _debugCommands;

    static void recvDebugDataMsg(uint8_t *data, size_t len);

    static void printSerialPort(String& s, LogLevel level);
    static void printSerialPort(const char *m, LogLevel level);
    static void printSerialPort(char *m, LogLevel level);
    static void printSerialPort(int m, LogLevel level);
    static void printSerialPort(uint8_t m, LogLevel level);
    static void printSerialPort(uint16_t m, LogLevel level);
    static void printSerialPort(uint32_t m, LogLevel level);
    static void printSerialPort(double m, LogLevel level);
    static void printSerialPort(float m, LogLevel level);

    static void printlnSerialPort(String& s, LogLevel level);
    static void printlnSerialPort(const char *, LogLevel level);
    static void printlnSerialPort(char *m, LogLevel level);
    static void printlnSerialPort(int m, LogLevel level);
    static void printlnSerialPort(uint8_t m, LogLevel level);
    static void printlnSerialPort(uint16_t m, LogLevel level);
    static void printlnSerialPort(uint32_t m, LogLevel level);
    static void printlnSerialPort(float m, LogLevel level);
    static void printlnSerialPort(double m, LogLevel level);

    static void printWifi(String& m, LogLevel level);
    static void printWifi(const char* m, LogLevel level);
    static void printWifi(char* m, LogLevel level);
    static void printWifi(int m, LogLevel level);
    static void printWifi(uint8_t m, LogLevel level);
    static void printWifi(uint16_t m, LogLevel level);
    static void printWifi(uint32_t m, LogLevel level);
    static void printWifi(double m, LogLevel level);
    static void printWifi(float m, LogLevel level);

    static void printlnWifi(String& m, LogLevel level);
    static void printlnWifi(const char* m, LogLevel level);
    static void printlnWifi(char* m, LogLevel level);
    static void printlnWifi(int m, LogLevel level);
    static void printlnWifi(uint8_t m, LogLevel level);
    static void printlnWifi(uint16_t m, LogLevel level);
    static void printlnWifi(uint32_t m, LogLevel level);
    static void printlnWifi(float m, LogLevel level);
    static void printlnWifi(double m, LogLevel level);

    static bool IsSerialOn();
    static bool IsWifiOn();
    static bool IsLevelOn(LogLevel level);

    static void InitializeCommands();

public:    
    static void begin(AsyncWebServer* server);
    static void print(String& m, LogLevel level);
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
    static void println(const char* m, LogLevel level);
    static void println(char* m, LogLevel level);
    static void println(int m, LogLevel level);
    static void println(uint8_t m, LogLevel level);
    static void println(uint16_t m, LogLevel level);
    static void println(uint32_t m, LogLevel level);
    static void println(float m, LogLevel level);
    static void println(double m, LogLevel level);
};

#endif