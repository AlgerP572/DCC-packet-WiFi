#include "NTPTimeClient.h"


WiFiUDP TimeClient::_ntpUDP;
NTPClient TimeClient::_timeClient(_ntpUDP);

void TimeClient::begin()
{
    _timeClient.begin();
}

bool TimeClient::update()
{
    return _timeClient.update();
}

bool TimeClient::forceUpdate()
{
    return _timeClient.forceUpdate();
}

void TimeClient::getFormattedDate(String& formattedTime)
{
    formattedTime = _timeClient.getFormattedDate();
}

void TimeClient::setTimeOffset(int timeOffset)
{
    _timeClient.setTimeOffset(timeOffset);
}