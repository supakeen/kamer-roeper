#include <Arduino.h>

#include <ESP8266WiFI.h>
#include <MQTT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LineProtocol.h>

#define VCC 3300

#define MEASUREMENT_WINDOW 5000
#define MEASUREMENT_COUNT 50

void reconnect();

IPAddress mqttServer(192, 168, 1, 10);
WiFiClient wifiClient = WiFiClient();
MQTTClient mqtt;
OneWire oneWire(0);  // GPIO0 -> D3 has internal pullup
DallasTemperature sensors(&oneWire);

#define every(t) for (static uint32_t _lasttime; (uint32_t)((uint32_t) millis() - _lasttime) >= (t); _lasttime = millis())

void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.begin("X", "X");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.print("Connected, IP = ");
  Serial.println(WiFi.localIP());

  mqtt.begin("192.168.1.10", 1883, wifiClient);

  sensors.begin();
  sensors.setWaitForConversion(false);
}

void loop_temperature() {
  sensors.requestTemperatures();

  unsigned int timeout = millis() + 500;

  while(millis() < timeout && !sensors.isConversionComplete()) { };

  float temperatureC = sensors.getTempCByIndex(0);

  char str[256];
  snprintf(str, sizeof(str), "temperature,room=study value=%f", temperatureC);
  mqtt.publish("/sensor/temperature", str);
  Serial.println(str);
}

void loop()
{
  if (!mqtt.connected())
    mqtt.connect("");
  mqtt.loop();

  every(15000) loop_temperature();
}