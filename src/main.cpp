#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <InfluxDb.h>

#include "config.h"
#include "OTAUpdate.h"

Influxdb influx(INFLUXDB_HOST); // port defaults to 8086

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  //reset saved settings - for debug purposes
  //wifiManager.resetSettings();
  
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Weather Sensors Logger");

  //OTA Update initialization
  OTAUpdate.begin();
  
  //Serial.println("Ready");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  influx.setDbAuth(INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PASS);
}

void loop() {

  //handle OTA update
  OTAUpdate.handle();
  
  // create a measurement object
  InfluxData measurement(INFLUXDB_MEASUREMENT);
  //measurement.addTag("device", d2);
  measurement.addTag("sensor", "random");
  measurement.addValue("temperature", random(0, 40));
  measurement.addValue("humidity", random(0, 100));
  // write it into db
  influx.write(measurement);

  delay(5000);
}