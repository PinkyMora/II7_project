
#include <ESP8266WiFi.h>
#include "PubSubClient.h"
#include <ArduinoJson.h>
#include "src/actualiza_dual.h"
#include "src/pulsador_int.h" 

extern "C" {                          // Utiliza librerias definidas en C, por lo que se introducen con la palabra extern
  #include "user_interface.h"
  #include <espnow.h>
}

//-------- Customise the above values --------
#define SENSORTOPIC "II7/sensoresPlaza/EstadoPlazas"
#define CONEXIONTOPIC "II7/sensoresPlazas/conexion"
#define CONFIGTOPIC "II7/sensoresPlazas/config"
#define FOTATOPIC "II7/sensoresPlazas/FOTA"

uint8_t mac[] = {0x48, 0x3F, 0xDA, 0x0C, 0xB7, 0xCF};// your MACMORA--> 48:3F:DA:0C:B7:CF     MACDAVID--> 48:3F:DA:77:1F:67
void initVariant() {
  //wifi_set_macaddr(SOFTAP_IF, &mac[0]);
}


char *ssid      =  "vodafoneAAXFSN";  // "sagemcom1928";    // "infind";               // Set you WiFi SSID
char *password  =  "AfG7CZqGYRMJYRG3"; // "UMMZUJNLTY3EMN";  //"1518wifi";               // Set you WiFi password

const char* mqtt_server = "iot.ac.uma.es"; 
const char* mqtt_user = "II7";
const char* mqtt_pass = "18K7ok1q";

//IPAddress server(192, 168, 1, 210);   //MQTT Broker address

const char deviceTopic[] = "ESPNOW/";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

unsigned long lastFOTA = 0;
unsigned long frec_actualiza_FOTA = 2*1000*60; // 2 min
String deviceMac;

// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA { // packed significa que utilizará el menor espacio posible
 String chipID;
 bool ocupado;
 float temp;
 float humedad;
 int numPlaza; //EN CASO DE QUE UNA PLACA CONTROLE MAS DE UN SENSOR DE PROXIMIDAD PARA LAS PLAZAS 
 bool ok;
} sensorData;

volatile boolean haveReading = false;

unsigned long tiempo_Escucha = 15000;

unsigned long heartBeat;
unsigned long heartBeat1;



void setup() {
  Serial.begin(115200); 
  Serial.println();
  Serial.println();
  Serial.println("ESP_Now Controller");
  Serial.println();

  setup_pulsador_int(); // SETUP DE LA PULSACION DEL BOTON 

  WiFi.mode(WIFI_AP);   // MODO WIFI-- ACCESS POINT PARA ESPNOW
  Serial.print("This node AP mac: "); Serial.println(WiFi.softAPmacAddress());
  WiFi.disconnect();
  Serial.print("This node mac: "); Serial.println(WiFi.macAddress());

  initEspNow();  
  Serial.println("Setup done");
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
    Serial.println(sensorData.chipID);
    Serial.println("-------------------------");  

    haveReading = true;
  });
}

void loop() 
{
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
            if(pulsador_evento==HIGH)
            {
              pulsador_evento = LOW;
              if (pulsador_estado_int == LOW)
              {
                Serial.printf("Se detecto una pulsacion en %d ms, \n", pulsador_btn_baja);
              }
              else
              {
                Serial.printf("Termino la pulsacion, duracion de %d ms\n", pulsador_ultima_int-pulsador_btn_baja);
                if (pulsador_ultima_int-pulsador_btn_baja > 1500) // PULSACION LARGA --> ACTUALIZAMOS FOTA
                {
                    Serial.println("Actualización FOTA por pulsación");
                    setup_OTA();
                    lastFOTA = millis();
                    
                }
              }
            }
      }
      Serial.print("SALGO BUCLE");
      heartBeat = millis();
      client.disconnect();
      delay(200);
      ESP.restart(); // <----- Reboots to re-enable ESP-NOW
    }
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
    String clientId = "ESP8266-";
    clientId += String(ESP.getChipId());
    // Attempt to connect
    
    String output = "";
    StaticJsonDocument<64> docFalse;
    docFalse["CHIPID"] = "PSensores";
    docFalse["online"] = "false";
    serializeJson(docFalse, output);
    
      if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass,CONEXIONTOPIC,1,true,output.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      // Construcción del mensaje de estatus
      String output = "";
      StaticJsonDocument<64> docTrue;
      docTrue["CHIPID"] = "PSensores";
      docTrue["online"] = "true";
      serializeJson(docTrue, output);
      client.publish(CONEXIONTOPIC, output.c_str(),true);
      // ... and resubscribe
      client.subscribe(CONFIGTOPIC);
      client.subscribe(FOTATOPIC);
      
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void sendToBroker() {

  String espnow;
  StaticJsonDocument<128> doc;
  
  doc["chipID"] = sensorData.chipID;
  doc["NumPlaza"] = sensorData.numPlaza;
  doc["Ocupado"] = sensorData.ocupado;
  
  JsonObject DHT11 = doc.createNestedObject("DHT11");
  DHT11["Temperatura"] = sensorData.temp;
  DHT11["Humedad"] = sensorData.humedad;
  
  serializeJson(doc, espnow);
  Serial.println(espnow);
  publishMQTT(SENSORTOPIC,espnow);
}

void publishMQTT(String topic, String message) {
  Serial.println("Publish");
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.publish(SENSORTOPIC, message.c_str());
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

  if ((String)topic==FOTATOPIC){
    // instrucción de actualización independientemente del payload
    setup_OTA();
    lastFOTA = millis();
  } else if ((String)topic==CONFIGTOPIC){
    // Parámetros configurables:
    // frec_actualiza_FOTA

      // String input;
      StaticJsonDocument<96> docConfig;
      DeserializationError error = deserializeJson(docConfig, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      frec_actualiza_FOTA = docConfig["frec_FOTA"];
      tiempo_Escucha = docConfig["tiempo_Escucha"];
      Serial.print("Frecuencia de actualizacion FOTA cambiada a: ");
      Serial.print(frec_actualiza_FOTA);
      Serial.println(" milisegundos");
       Serial.print("Tiempo de escucha de topics MQTT establecido en: ");
      Serial.print(tiempo_Escucha);
      Serial.println(" milisegundos");
      
  }
}
