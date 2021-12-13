#include <ESP8266WiFi.h>
#include "src/JSONHandler.h"
#include "src/DHTHandler.h"
#include "src/MQTTHandler.h"

// Update these with values suitable for your network.
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "iot.ac.uma.es";
const char* mqtt_user = "infind";
const char* mqtt_pass = "zancudo";

WiFiClient espClient;
struct registro_datos misdatos;
OurDHT dht(5);
MQTTHandler mqtt(espClient, mqtt_server, mqtt_user, mqtt_pass);
unsigned long lastMsg;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  setup_wifi();
  Serial.println("Comenzamos...");
  lastMsg = millis();
}

void loop() {
  mqtt.loop();
  unsigned long now = millis();
  if (now - lastMsg > 60000) {
    dht.read(misdatos);
    Serial.println("JSON generado con ArduinoJson:");
    Serial.println(serializa_JSON_string(misdatos));
    Serial.println("Enviando por MQTT...");
    String mensaje = serializa_JSON_string(misdatos);
    Serial.println(mqtt.sendData(serializa_JSON_string(misdatos)));
    Serial.println("Enviado. Nos vemos en 1 minuto");
    lastMsg = now;
  }
  
  
}
