#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include "actualiza_dual.h"
#include "pulsador_int.h"
// -------------------  Configuración WiFi  ------------------- //

const char* ssid = "Redmi";
const char* password = "Redmi103";


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
Servo servo;
int angle=0;
const int pin_servo=5;

// Parámetros configurables mediante II7/ESP32/config
unsigned long frec_actualiza_FOTA = 1000*60*2;  // inicialmente 2 min
                                                // debe venir en un json
                                                // y ser un valor, no string
char mensaje[128];  // cadena de 128 caracteres

// ------- Función callback ------ //

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((String)topic=="II7/Entrada/FOTA"){
    // instrucción de actualización independientemente del payload
    setup_OTA();
    lastFOTA = millis();
  } else if ((String)topic=="II7/Entrada/config"){                                  // Parámetros configurables:
                                                                                    // frec_actualiza_FOTA
      // String input;
      StaticJsonDocument<48> docFrecFOTA;
      DeserializationError error = deserializeJson(docFrecFOTA, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }
      frec_actualiza_FOTA = docFrecFOTA["frec_FOTA"];                               // Frecuencia de actualizacion FOTA cambiada segun la información recibida
      Serial.print("Frecuencia de actualizacion FOTA cambiada a: ");
      Serial.print(frec_actualiza_FOTA);
      Serial.println(" milisegundos");
  }
  else if ((String)topic=="II7/Entrada/BarreraCMD"){                                // Topic relativo a las instrucciones de la barrera
    
    StaticJsonDocument<48> docBarrera;
    DeserializationError error = deserializeJson(docBarrera, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    String comando = docBarrera["comando"];
    
    if (comando=="subir"){                                                          // Instrucción "subir" obtenida
      servo.write(90);                                                              // Barrera sube a 90º
      client.publish("II7/Entrada/BarreraEstado","{\"estado\":\"subido\"}",true);   // Publica en el topic "II7/Entrada/BarreraEstado" que su estado es "subido"
    }
    
    else if (comando=="bajar"){                                                     // Instrucción "bajar" obtenida
      for(angle=servo.read();angle>=0;angle=angle-10){                              // Barrera baja lentamente hasta 0º, para evitar accidentes
      servo.write(angle);                                                     
      delay(500);
      }
      servo.write(0);                                                               // En caso de cualquier error, asegura que la barrera baja completamente
      client.publish("II7/Entrada/BarreraEstado","{\"estado\":\"bajado\"}",true);   // Publica en el topic "II7/Entrada/BarreraEstado" que su estado es "bajado"
    }
  }
}

// ------- Función reconexión ------- //

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("\nAttempting MQTT connection...");
    // Create a client ID
    String clientId = "ESP8266Client-";
    clientId += String(ESP.getChipId());
    // Attempt to connect

    // Construcción dell mensaje de estatus
    String output = "";
    StaticJsonDocument<64> docFalse;
    docFalse["CHIPID"] = "Entrada";
    docFalse["online"] = "false";
    serializeJson(docFalse, output);
    
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass,"II7/Entrada/conexion",1,true,output.c_str())) { // Definición de LWM en modo retenido
      Serial.println("connected");
      // Once connected, publish an announcement...
      // Construcción dell mensaje de estatus
      String output = "";
      StaticJsonDocument<64> docTrue;
      docTrue["CHIPID"] = "Entrada";
      docTrue["online"] = "true";
      serializeJson(docTrue, output);
      // -------------------------------------
      client.publish("II7/Entrada/conexion",output.c_str(),true); // Mensaje de aviso de conexion en modo retenido
      // ... and resubscribe to topics
      client.subscribe("II7/Entrada/FOTA",1);                     // Topic empleado para la actualiacion FOTA
      client.subscribe("II7/Entrada/config",1);                   // Topic empleado para configurar parámetros de la placa de manera externa
      client.subscribe("II7/Entrada/BarreraCMD",1);               // Topic empleado para recibir instrucciones sobre el comportamiento de la barrera
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
  client.setCallback(callback);
}

// -------------------------------------------------------//
// -------------------------------------------------------//
// -------------------------------------------------------//

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  setup_OTA();                        // Se realiza un primer intento de actualización FOTA
  lastFOTA = millis();                // Se registra el momento en el que fue intentado por última vez
  setup_MQTT();                       // Se procede con el intento de conexión al broker MQTT

  servo.attach(pin_servo,380,2180);   // Asigna al objeto servo el pin y los valores de los pulsos en microsegundos para 0 y 180º
  servo.write(angle);                 // Inicializa el servo al valor inicial de angulo (90º)
  
  setup_pulsador_int();               // Llama a la función que habilita el funcionamiento del botón 
}

void loop() {
  if (!client.connected()) {
    reconnect();                      // Si la placa no se ha conectado al servidor MQTT, vuelve a intentarlo
  }
  client.loop();

  unsigned long now = millis();       
  
  if ((now - lastFOTA) > (frec_actualiza_FOTA)){          // Si ha pasado el tiempo especificado entre intentos de actualización, se vuelve a intentar
    setup_OTA();
    lastFOTA = now;
  }

  if(pulsador_evento==HIGH)                               // Se ha pulsado/soltado el botón
  {
    pulsador_evento = LOW;                                // Una vez detectado el evento, se pone la variable a LOW
    if (pulsador_estado_int == LOW)                       // LOW = Botón pulsado, al detectarse un evento y el botón pulsado, se sabe que se acaba de pulsar
    {
      Serial.printf("Se detecto una pulsacion en %d ms, \n", pulsador_btn_baja);
    }
    else                                                  // Si se detecta un evento en el botón pero su estado es HIGH (botón sin pulsar), se sabe que estaba pulsado y se ha soltado
    {                                                     // Por tanto, la pulsación ha terminado
      
      Serial.printf("Termino la pulsacion, duracion de %d ms\n", pulsador_ultima_int-pulsador_btn_baja);
      
      if ((pulsador_ultima_int-pulsador_btn_baja)>3000){  // Se observa el tiempo que ha estado pulsado, si es mayor a 3s, se trata de actualizar la placa (Actualización manual)
        setup_OTA();
        lastFOTA = now;
      }
      else{                                               // Si el tiempo pulsado es menor a 3s, es porque un vehículo quiere acceder al garaje
        Serial.println("Nuevo vehículo detectado");
        client.publish("II7/Entrada/Pulsador", "1");      // Se envía un mensaje al topic "II7/Entrada/Pulsador" con el valor "1", indicando que el botón ha sido pulsado
      }                                                   // La placa se mantendrá entonces a la espera de recibir la confirmación por parte del flujo de NodeRed para
    }                                                     // levantar la barrera
  }

  
    
   

  
}
