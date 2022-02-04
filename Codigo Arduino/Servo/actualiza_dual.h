#ifdef ESP32    // librerías ESP32
  #include <HTTPUpdate.h>
  #include <WiFi.h>
  #include <HTTPClient.h>
#else           // librerías ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266httpUpdate.h>
#endif

void setup_OTA();
void setup_wifi(const char*, const char*);

#ifndef ESP32
  void progreso_OTA(int,int);
  void final_OTA();
  void inicio_OTA();
  void error_OTA(int);
#endif
