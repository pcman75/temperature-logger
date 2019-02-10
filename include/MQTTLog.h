#ifndef __MQTTLog_H
#define __MQTTLog_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MQTTLog
{
  public:
    MQTTLog(IPAddress borkerIP);
    ~MQTTLog();

    void print(const char *msg);
    void print(const String& msg);
    void printf(const char *format, ...);
    
  private:
    boolean reconnect();

  private:
    WiFiClient _wclient;
    PubSubClient _client = PubSubClient(_wclient); // Setup MQTT client

    long _lastReconnectAttempt = 0;
};

#endif
