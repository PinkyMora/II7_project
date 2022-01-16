/**
 * 
 * Author: Andreas Spiess, 2017
 * Slightly adapted by Diy_bloke 2020
  This sketch receives ESP-Now message and sends it as an MQTT messge
  It is heavily based on of Anthony's gateway setch sketch

https://github.com/HarringayMakerSpace/ESP-Now
Anthony Elder
 */
#include <ESP8266WiFi.h>
#include "PubSubClient.h"
//#include "src/JSONHandler.h"
#include <ArduinoJson.h>
//#include "actualiza_dual.h"

extern "C" {                          // Utiliza librerias definidas en C, por lo que se introducen con la palabra extern
  #include "user_interface.h"
  #include <espnow.h>
}


//-------- Customise the above values --------
#define SENSORTOPIC "II7/ESP8266/ESPNOW"
#define COMMANDTOPIC "infind/ESPNow/command"
#define SERVICETOPIC "infind/ESPNow/service"

/* Set a private Mac Address
 *  http://serverfault.com/questions/40712/what-range-of-mac-addresses-can-i-safely-use-for-my-virtual-machines
 * Note: the point of setting a specific MAC is so you can replace this Gateway ESP8266 device with a new one
 * and the new gateway will still pick up the remote sensors which are still sending to the old MAC 
 */
uint8_t mac[] = {0x48, 0x3F, 0xDA, 0x0C, 0xB7, 0xCF};// your MACMORA--> 48:3F:DA:0C:B7:CF     MACDAVID--> 48:3F:DA:77:1F:67
void initVariant() {
  //wifi_set_macaddr(SOFTAP_IF, &mac[0]);
}


char *ssid      =  "vodafoneAAXFSN";  // "sagemcom1928";    // "infind";               // Set you WiFi SSID
char *password  =  "AfG7CZqGYRMJYRG3"; // "UMMZUJNLTY3EMN";  //"1518wifi";               // Set you WiFi password

const char* mqtt_server = "iot.ac.uma.es";  //"iot.ac.uma.es";
const char* mqtt_user = "II7";
const char* mqtt_pass = "18K7ok1q";

//IPAddress server(192, 168, 1, 210);   //your MQTT Broker address

const char deviceTopic[] = "ESPNOW/";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

unsigned long lastFOTA = 0;
unsigned long frec_actualiza_FOTA = 2*1000*60; // 2 min
String deviceMac;

// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA { // packed significa que utilizará el menor espacio posible
 bool ocupado;
 float temp;
 float humedad;
 int numPlaza; //EN CASO DE QUE UNA PLACA CONTROLE MAS DE UN SENSOR DE PROXIMIDAD PARA LAS PLAZAS 
 bool ok;
} sensorData;

volatile boolean haveReading = false;

/* Presently it doesn't seem posible to use both WiFi and ESP-NOW at the
 * same time. This gateway gets around that be starting just ESP-NOW and
 * when a message is received switching on WiFi to sending the MQTT message
 * to Watson, and then restarting the ESP. The restart is necessary as 
 * ESP-NOW doesn't seem to work again even after WiFi is disabled.
 * Hopefully this will get fixed in the ESP SDK at some point.
 */
unsigned long tiempo_Escucha = 15000;

unsigned long heartBeat;
unsigned long heartBeat1;



void setup() {
  Serial.begin(115200); 
  Serial.println();
  Serial.println();
  Serial.println("ESP_Now Controller");
    Serial.println();

  WiFi.mode(WIFI_AP);
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  WiFi.disconnect();
  Serial.print("This node mac: "); Serial.println(WiFi.macAddress());

  initEspNow();  
  Serial.println("Setup done");
}


void loop() {
  if (millis()-heartBeat > 30000) {
    Serial.println("Waiting for ESP-NOW messages...");
    heartBeat = millis();
  }

  if (haveReading) {
    
    haveReading = false;
    wifiConnect();
    client.setServer(mqtt_server, 1883);
    reconnectMQTT();
    client.setCallback(callback);
    
    sendToBroker(); //  SI VAMOS A RECIBIR DATOS POR MQTT DE ACTUS O ALGO DEBERÍA ENTRAR AQUI Y LLAMAR AL 
                    // CALLBACK ADEMÁS DE SUBSCRIBIRNOS AL TOPIC PERTINENTE
    heartBeat1 = millis();
    while (millis()-heartBeat1 < tiempo_Escucha){
      client.loop();
    }
    Serial.print("SALGO BUCLE");
    heartBeat = millis();
    client.disconnect();
    delay(200);
    ESP.restart(); // <----- Reboots to re-enable ESP-NOW
  }
}
void publishMQTT(String topic, String message) {
  Serial.println("Publish");
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.publish(SENSORTOPIC, message.c_str());
}

void sendToBroker() {


  StaticJsonDocument<128> doc;
  String espnow;
  doc["Ocupado"] = sensorData.ocupado;
  doc["Temperatura"] = sensorData.temp;
  doc["Humedad"] = sensorData.humedad;
  doc["NumPlaza"] = sensorData.numPlaza;
  doc["status"] = sensorData.ok;
  
  serializeJson(doc, espnow);
  Serial.println(espnow);
  publishMQTT(SENSORTOPIC,espnow);
}

void initEspNow() {
  if (esp_now_init()!=0) {
    Serial.println("*** ESP_Now init failed");
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // Rol de master y de esclavo --> COMBO

  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {

    deviceMac = "";
    deviceMac += String(mac[0], HEX);
    deviceMac += String(mac[1], HEX);
    deviceMac += String(mac[2], HEX);
    deviceMac += String(mac[3], HEX);
    deviceMac += String(mac[4], HEX);
    deviceMac += String(mac[5], HEX);
    
    memcpy(&sensorData, data, sizeof(sensorData));

    Serial.print("Message received from device: "); 
    Serial.println(deviceMac);
    Serial.printf(" Plaza=%f, Status=%f\n", 
       sensorData.ocupado, sensorData.ok);    

    haveReading = true;
  });
}

void wifiConnect() {
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     delay(250);
     Serial.print(".");
  }  
  Serial.print("\nWiFi connected, IP address: "); Serial.println(WiFi.localIP());
}



void reconnectMQTT() {
  Serial.println(" Loop until we're reconnected");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP-NOWClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    
      if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("II7/ESP8266/conexion", "I am live");
      // ... and resubscribe
      client.subscribe("II7/ESP8266/config");
      //client.subscribe("II7/ESP8266/FOTA");
      
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// ------- Función callback ------ //

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((String)topic=="II7/ESP8266/FOTA"){
    // instrucción de actualización independientemente del payload
    //setup_OTA();
    lastFOTA = millis();
  } else if ((String)topic=="II7/ESP8266/config"){
    // Parámetros configurables:
    // frec_actualiza_FOTA

      // String input;
      StaticJsonDocument<48> docFrecFOTA;
      DeserializationError error = deserializeJson(docFrecFOTA, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      frec_actualiza_FOTA = docFrecFOTA["frec_FOTA"];
      tiempo_Escucha = docFrecFOTA["tiempo_Escucha"];
      Serial.print("Frecuencia de actualizacion FOTA cambiada a: ");
      Serial.print(frec_actualiza_FOTA);
      Serial.println(" milisegundos");
       Serial.print("Tiempo de escucha de topics MQTT establecido en: ");
      Serial.print(tiempo_Escucha);
      Serial.println(" milisegundos");
      
  }
}
