#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include "actualiza_dual.h"
#include <Servo.h>
#include "pulsador_int.h"
// -------------------  Configuración WiFi  ------------------- //

//const char* ssid = "vodafoneAAXFSN";
//const char* password = "AfG7CZqGYRMJYRG3";
//const char* ssid = "infind";
//const char* password = "1518wifi";
char* ssid = "Redmi";
char* password = "Redmi103";
//const char* ssid = "Lecom-e0-31-0E";
//const char* password = "athoo5ooJai6aif8";
//const char* ssid = "Lecom-Fibra-69-99-d6";
//const char* password = "fie6roh5Xah9ua4D";
//const char* ssid = "OnePlus Nord2 5G";
//const char* password = "eo792ndpw75kw";


// ---------------------------------------------------------- //
// ------------------- Configuración MQTT ------------------- //
// ---------------------------------------------------------- //

const char* mqtt_server = "iot.ac.uma.es";
const char* mqtt_user = "II7";
const char* mqtt_pass = "18K7ok1q";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastFOTA = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Parámetros servo
Servo servo_in;
Servo servo_out;
int angle_in=0;
int angle_out=0;
const int pin_servo_in=5;
const int pin_servo_out=4;

// Parámetros configurables mediante II7/ESP32/config
unsigned long frec_actualiza_FOTA = 1000*60*2;  // inicialmente 2 min
                                                // debe venir en un json
                                                // y ser un valor, no string
char mensaje[128];  // cadena de 128 caracteres

// ------- Función callback ------ //

//void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();
//
//  if ((String)topic=="II7/ESP32/FOTA"){
//    // instrucción de actualización independientemente del payload
//    setup_OTA();
//    lastFOTA = millis();
//  } else if ((String)topic=="II7/ESP32/config"){
//    // Parámetros configurables:
//    // frec_actualiza_FOTA
//
//      // String input;
//      StaticJsonDocument<48> docFrecFOTA;
//      DeserializationError error = deserializeJson(docFrecFOTA, payload);
//      if (error) {
//        Serial.print("deserializeJson() failed: ");
//        Serial.println(error.c_str());
//        return;
//      }
//      frec_actualiza_FOTA = docFrecFOTA["frec_FOTA"];
//      Serial.print("Frecuencia de actualizacion FOTA cambiada a: ");
//      Serial.print(frec_actualiza_FOTA);
//      Serial.println(" milisegundos");
//  }
//}

// ------- Función reconexión ------- //

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("\nAttempting MQTT connection...");
    // Create a client ID
    String clientId = "ESP32Client";
    //clientId += String(ESP.getChipId());  // .getChipId() no funciona con ESP32
    // Attempt to connect

    // Construcción dell mensaje de estatus
    String output = "";
    StaticJsonDocument<64> docFalse;
    docFalse["CHIPID"] = "ESP32";
    docFalse["online"] = "false";
    serializeJson(docFalse, output);
    
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass,"II7/ESP32/conexion",1,true,output.c_str())) { // Definición de LWM en modo retenido
      Serial.println("connected");
      // Once connected, publish an announcement...
      // Construcción dell mensaje de estatus
      String output = "";
      StaticJsonDocument<64> docTrue;
      docTrue["CHIPID"] = "ESP32";
      docTrue["online"] = "true";
      serializeJson(docTrue, output);
      // -------------------------------------
      client.publish("II7/ESP32/conexion",output.c_str(),true); // Mensaje de aviso de conexion en modo retenido
      // ... and resubscribe to topics
      client.subscribe("II7/ESP32/FOTA",1);
      client.subscribe("II7/ESP32/config",1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// -------- Función de inicialización ---------- //

void setup_MQTT()
{
  client.setServer(mqtt_server, 1883);
//  client.setCallback(callback);
}

// -------------------------------------------------------//
// -------------------------------------------------------//
// -------------------------------------------------------//

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  WiFi.begin(ssid, password);

//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
//  Serial.println("WiFi connected");

//  setup_OTA();
//  lastFOTA = millis();
  setup_MQTT();

  Serial.print("sin actualizar");

  servo_in.attach(pin_servo_in,380,2180); // Asigna al objeto servo el pin y los valores de los pulsos en microsegundos para 0 y 180º
  servo_in.write(angle_in);              // Inicializa el servo al valor inicial de angulo (90º)

  servo_out.attach(pin_servo_out,450,2300);
  servo_out.write(angle_out);

  setup_pulsador_int();             
}

void nuevo_vehiculo(){
  
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  
//  if ((now - lastFOTA) > (frec_actualiza_FOTA)){
//    setup_OTA();
//    lastFOTA = now;
//    Serial.println("Actualizado!!!!");
//  }

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
      if ((pulsador_ultima_int-pulsador_btn_baja)>3000){
//        setup_OTA();
//        lastFOTA = now;
        Serial.println("Actualizado!!!!");
      }
      else{
        nuevo_vehiculo();
        Serial.println("Nuevo vehículo detectado"); 
        
//        DynamicJsonDocument doc(1024);
//        doc["sensor"] = "gps";
//        doc["time"]   = 1351824120;
//        doc["data"][0] = 48.756080;
//        doc["data"][1] = 2.302038;
//        
//        serializeJson(doc, Serial);
        client.publish("II7/ESP8266/Pulsador", "1");
        delay(5000);
        servo_in.write(90);
        delay(5000);
        for(angle_in=90;angle_in>=0;angle_in=angle_in-10){
          servo_in.write(angle_in);
          delay(500);
        }
      }
    }
  }

  
    
   

  
}
