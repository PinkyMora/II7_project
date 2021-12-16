#include "JSONHandler.h"


String serializa_JSON_string (struct registro_datos datos)
{
  StaticJsonDocument<300> jsonRoot;
  String jsonString;
 
  jsonRoot["Uptime"]= datos.tiempo;
  JsonObject DHT11=jsonRoot.createNestedObject("DHT11");
  DHT11["temp"] = datos.temperatura;
  DHT11["hum"] = datos.humedad;
  
  
  serializeJson(jsonRoot,jsonString);
  return jsonString;
}

const char* serializa_JSON_char (struct registro_datos datos)
{
  return serializa_JSON_string(datos).c_str();
}
