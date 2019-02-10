#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <InfluxDb.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#include "config.h"
#include "OTAUpdate.h"
#include "MQTTLog.h"

Influxdb influx(INFLUXDB_HOST); // port defaults to 8086
Adafruit_BME280 bme;            // I2C
#define SEALEVELPRESSURE_HPA (1024.99)

MQTTLog Log = MQTTLog(MQTT_BROKER_IP);

void readTemperatureAndPublish();
Ticker bmeTicker(readTemperatureAndPublish, 10000);

void readTemperatureAndPublish()
{
  // Only needed in forced mode! In normal mode, you can remove the next line.
  bme.takeForcedMeasurement(); // has no effect in normal mode

  // create a measurement object
  InfluxData measurement(INFLUXDB_MEASUREMENT);

  measurement.addTag("sensor", "bme280");
  String chipId = String(ESP.getChipId(), HEX);
  measurement.addTag("device", chipId.c_str());
  measurement.addTag("mode", "weather");

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  measurement.addValue("Temperature", temperature);
  measurement.addValue("Humidity", humidity);
  measurement.addValue("Pressure", pressure);
  measurement.addValue("Altitude", altitude);

  // write it into db
  influx.write(measurement);
  Log.printf("Temperature = %.2f, Humidity = %.2f, Pressure = %.2f, Altitude = %.2f", temperature, humidity, pressure, altitude);
}

void setup()
{
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
  if (!bme.begin(0x76))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF);

  influx.setDbAuth(INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PASS);

  //test
  Serial.println(WiFi.localIP().toString());

  // read the temperature every 10s
  bmeTicker.start();
}

void loop()
{

  //handle OTA update
  OTAUpdate.handle();
  bmeTicker.update();
}