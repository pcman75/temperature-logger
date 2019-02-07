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
#define SEALEVELPRESSURE_HPA (1024.99)

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
  
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );

  influx.setDbAuth(INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PASS);
}

void loop() {

  //handle OTA update
  OTAUpdate.handle();

  // Only needed in forced mode! In normal mode, you can remove the next line.
  bme.takeForcedMeasurement(); // has no effect in normal mode
    
  // create a measurement object
  InfluxData measurement(INFLUXDB_MEASUREMENT);

  measurement.addTag("sensor", "bme280");
  static char buf[10];
  itoa(system_get_chip_id(), buf, HEX);
  measurement.addTag("device", buf);
  measurement.addTag("mode", "weather");

  measurement.addValue("Temperature", bme.readTemperature());
  measurement.addValue("Humidity", bme.readHumidity());
  measurement.addValue("Altitude", bme.readPressure() / 100.0F);
  measurement.addValue("Altitude", bme.readAltitude(SEALEVELPRESSURE_HPA));

  // write it into db
  influx.write(measurement);
  
  delay(60000);
}