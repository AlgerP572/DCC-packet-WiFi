
#include "WebServer.h"
#include "WifiConnection.h"
#include "WifiSerialDebug.h"

bool WifiConnection::setup(AsyncWebServer* server, DNSServer* dns, bool reset)
{ 
    if (server == nullptr)
    {
        // Handle the error appropriately        
        Log::println("Error: Server instance is null", LogLevel::ERROR);
        return false;
    }

    if (dns == nullptr)
    {
        // Handle the error appropriately       
        Log::println("Error: dns instance is null", LogLevel::ERROR);
        return false;
    }

    AsyncWiFiManager wm(server, dns);
    if(reset)
    {

        // reset settings - wipe stored credentials for testing
        // these are stored by the esp library
        wm.resetSettings();
    }

    // The decoder needs to know the setup of the Wifi network so it can construct its
    // static address properly.  We first autoconnect discover the gateway IP.
    // It is important that the WiFi network router is using a subnet mask of
    // 255.255.0.0.  This allows for auto translation from the DCC address to the
    // DCC over Wifi address. Since the ip address segments are only bytes (i.e. 0-255)
    // and DCC supports ~10000 address, the first two numbers of the adress appear in the
    // third segment of the ip address with a first digit of 1 and the second two numbers
    // of the dcc address are encoded in the fourth segment of the ip address also using
    // a 1 for the first digit. examples.
    //
    // The first two segments are determined by your WiFi router and are discovered at
    // runtime.  The examples assume thew WiFi is using gateway 10.0.0.1 and
    // subnet mask 255.255.0.0
    //
    //  DCC Address             DCC over Wifi Address
    //  3                       10.0.100.103
    //  14                      10.0.100.114
    //  415                     10.0.104.115
    //  3985                    10.0.139.185
    return wm.autoConnect("Wireless-DCC-Decoder-Get-Setup");

// TODO: still a work in progess...
//     IPAddress gateway2 = WiFi.gatewayIP();
//     byte first = gateway2[0];
//     WiFi.disconnect();

//     // set custom static ip for portal
//     IPAddress staticIP(10,0,100,120);  //ESP static ip
//     IPAddress gateway(10,0,0,1); //IP Address of your WiFi Router (Gateway)
//     IPAddress subnet(255,255,0,0); //Subnet mask
//     wm.setSTAStaticIPConfig(staticIP, gateway, subnet);
   
//    return wm.autoConnect("Wireless-DCC-Decoder"); // auto generated AP name from chipid  
}
