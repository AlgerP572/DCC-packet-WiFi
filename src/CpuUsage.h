#ifndef CpuUsage_h
#define CpuUsage_h

#include <Arduino.h>

class CpuUsage
{ 
public:
    static void setup();

    static float GetCpuLoad();    

private:    
    static float _cpuLoad;   

    static void idleTask(void* data);
    static void measureTask(void* data);   
};

#endif