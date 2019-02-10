#include <PubSubClient.h>

#include "MQTTLog.h"

MQTTLog::MQTTLog(IPAddress brokerIP)
{
    _client.setServer(brokerIP, 1883);
}

MQTTLog::~MQTTLog()
{
}

void MQTTLog::print(const String& msg)
{
    return print(msg.c_str());
}

void MQTTLog::print(const char *msg)
{
    if (!_client.connected())
    {
        long now = millis();
        if (now - _lastReconnectAttempt > 5000)
        {
            _lastReconnectAttempt = now;
            // Attempt to reconnect
            if (reconnect())
            {
                _lastReconnectAttempt = 0;
            }
        }
    }
    else
    {
        // Client connected
        _client.loop();
        // Once connected, publish te message
        _client.publish("DeviceDebug", msg);
        Serial.println(msg);
    }
}

void MQTTLog::printf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char* buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
        buffer = new char[len + 1];
        if (!buffer) {
            return;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
    }
    print(buffer);
    if (buffer != temp) {
        delete[] buffer;
    }
    return;
}

boolean MQTTLog::reconnect()
{
    String chipId = String(ESP.getChipId(), HEX);
    if (_client.connect(chipId.c_str()))
    {
        _client.publish("outTopic", (chipId + " MQTT reconnected").c_str());
        Serial.println(chipId + " MQTT reconnected");
    }
    return _client.connected();
}