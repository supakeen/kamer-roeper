#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define VCC 3300

#define MEASUREMENT_WINDOW 5000
#define MEASUREMENT_COUNT 50

void callback(char*, byte*, unsigned int);
void reconnect();

IPAddress mqttServer(192, 168, 1, 10);
WiFiClient wifiClient = WiFiClient();
PubSubClient mqttClient(mqttServer, 1883, callback, wifiClient);

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
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  
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

  snprintf(str, sizeof(str), "temperature,room=livingroom value=%f", avg);

  mqttClient.publish("/esp8266/temperature", str);

  Serial.println(avg);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Callback");
  Serial.println((char) payload[0]);
}

void reconnect() {
 Serial.println("Connecting to MQTT Broker...");
 while (!mqttClient.connected()) {
     Serial.println("Reconnecting to MQTT Broker..");
     String clientId = "esp8266-livingroom-a";
     clientId += String(random(0xffff), HEX);
    
     if (mqttClient.connect(clientId.c_str())) {
       Serial.println("Connected.");
       // subscribe to topic      
     }
 }
}
