
#include "WebServer.h"

AsyncWebServer WebServer::_server(80);
DNSServer WebServer::_dns;

 AsyncWebServer* WebServer::get()
 {
    return &_server;
 }

 DNSServer* WebServer::dns()
 {
    return &_dns;
 }

 void WebServer::begin()
 {
    _server.begin();
 }