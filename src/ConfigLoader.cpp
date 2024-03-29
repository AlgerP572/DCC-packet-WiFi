#include <SPIFFS.h>

#include "ConfigLoader.h"
#include "WifiSerialDebug.h"

bool ConfigLoader::readFile(const String& fileName,  String& buffer)
{
    if (!SPIFFS.exists(fileName))
    {       
        Log::print("File" + fileName + " not found\n", LogLevel::ERROR);
        return false;
    }

    File dataFile = SPIFFS.open(fileName, "r");
    if (!dataFile)
    {        
        Log::print("Can't open " + fileName +" \n", LogLevel::ERROR);
        return false;
    }

    buffer.reserve(dataFile.size() + 1);
    while (dataFile.available())
    {
        buffer += dataFile.readString();
    }

    dataFile.close();
    return true;
}

JsonDocument* ConfigLoader::getDocPtr(const String& cmdFile)
{
    String jsonData;
    
    readFile(cmdFile, jsonData);    
    if (jsonData.length() <= 0)
    {
        Log::println("File read error", LogLevel::ERROR);
        return NULL;
    }

    uint16_t docSize = 4096 * (trunc((3 * jsonData.length()) / 4096) + 1);
    Log::print("Size:" + jsonData +  "Doc Size:" + docSize + "\n", LogLevel::DEBUG);
    DynamicJsonDocument * thisDoc = new DynamicJsonDocument(docSize);
    DeserializationError error;
    error = deserializeJson(*thisDoc, jsonData); 
        
    if (error)
    {
        Log::println("Deserialization error.", LogLevel::ERROR);
        return NULL;
    } 

    return thisDoc; 
}