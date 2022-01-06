#include "esp_camera.h"
#include <WiFi.h>
#include "actualiza_dual.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

// QUEDA EL CALLBACK DE TOPIC CONFIG !!!!

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

// -------------------  Configuración WiFi  ------------------- //

//const char* ssid = "vodafoneAAXFSN";
//const char* password = "AfG7CZqGYRMJYRG3";
//const char* ssid = "infind";
//const char* password = "1518wifi";
//const char* ssid = "Redmi";
//const char* password = "mamawevo";
const char* ssid = "Lecom-e0-31-0E";
const char* password = "athoo5ooJai6aif8";
//const char* ssid = "Lecom-Fibra-69-99-d6";
//const char* password = "fie6roh5Xah9ua4D";
//const char* ssid = "OnePlus Nord2 5G";
//const char* password = "1234567890";


// ---------------------------------------------------------- //
// ------------------- Configuración MQTT ------------------- //
// ---------------------------------------------------------- //

const char* mqtt_server = "";
const char* mqtt_user = "";
const char* mqtt_pass = "";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastFOTA = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

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

  if ((String)topic=="II7/ESP32/FOTA"){
    // instrucción de actualización independientemente del payload
    setup_OTA();
    lastFOTA = millis();
  } else if ((String)topic=="II7/ESP32/config"){
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
  }
}

// ------- Función reconexión ------- //

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a client ID
    String clientId = "ESP32Client";
    //clientId += String(ESP.getChipId());  // .getChipId() no funciona con ESP32
    // Attempt to connect

    // Construcción dell mensaje de estatus
    String output = "";
    StaticJsonDocument<64> doc;
    doc["CHIPID"] = "ESP32";
    doc["online"] = "false";
    serializeJson(doc, output);
    
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass,"II7/ESP32/conexion",1,true,output.c_str())) { // Definición de LWM en modo retenido
      Serial.println("connected");
      // Once connected, publish an announcement...
      doc["online"] = "true";
      serializeJson(doc, output);
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
  client.setCallback(callback);
}

// -------------------------------------------------------//
// -------------------------------------------------------//
// -------------------------------------------------------//



void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  //setup_wifi(ssid,password); // tiene que hacerse con el código del ejemplo para
                               // montar el servidor http de la cámara
  setup_OTA();
  lastFOTA = millis();
  setup_MQTT();

  Serial.print("sin actualizar");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  
    if ((now - lastFOTA) > (1000*60*2.5)){
      setup_OTA();
      lastFOTA = now;
    }

    

  
}
