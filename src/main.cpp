#include <Arduino.h>
#include <SPI.h>

#include <ESP8266WiFI.h>
#include <MQTT.h>
#include <LineProtocol.h>

#define VCC 3300

#define MEASUREMENT_WINDOW 5000
#define MEASUREMENT_COUNT 50

void reconnect();

IPAddress mqttServer(192, 168, 1, 10);
WiFiClient wifiClient = WiFiClient();
MQTTClient mqtt;

const int PIN_TMP36 = A0;

float temperatures[MEASUREMENT_COUNT] = { 0 };

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.begin("X", "X");

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP = ");
  Serial.println(WiFi.localIP());

  mqtt.begin("192.168.1.10", 1883, wifiClient);
}

void loop() {
  if (!mqtt.connected()) mqtt.connect("");
  mqtt.loop();
  
  float sum = 0;
  float avg = 0;
  char str[128] = { 0 };
  
  for(int i = 0; i < MEASUREMENT_COUNT; i++) {
    temperatures[i] = (map(analogRead(PIN_TMP36), 0, 1023, 0, VCC) - 500) / 10;
    delay(MEASUREMENT_WINDOW / MEASUREMENT_COUNT);
  };

  for(int i = 0; i < MEASUREMENT_COUNT; i++) {
    sum += temperatures[i];
  }

  avg = sum / MEASUREMENT_COUNT;

  snprintf(str, sizeof(str), "temperature,room=study value=%f", avg);

  mqtt.publish("/sensor/temperature", str);

  Serial.println(avg);
}