#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <InfluxDb.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "config.h"
#include "OTAUpdate.h"

Influxdb influx(INFLUXDB_HOST); // port defaults to 8086
Adafruit_BME280 bme; // I2C
#define SEALEVELPRESSURE_HPA (1013.25)

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
  
  Serial.println(F("BME280 test"));

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  if (!bme.begin(0x76)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }
    
  influx.setDbAuth(INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PASS);
}

void printValues() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}

void loop() {

  //handle OTA update
  OTAUpdate.handle();
  
  printValues();

  /*
  // create a measurement object
  InfluxData measurement(INFLUXDB_MEASUREMENT);
  //measurement.addTag("device", d2);
  measurement.addTag("sensor", "random");
  measurement.addValue("temperature", random(0, 40));
  measurement.addValue("humidity", random(0, 100));
  // write it into db
  influx.write(measurement);
  */

  delay(6000);
}