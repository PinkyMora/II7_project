#ifndef JSONHANDLER_H_
#define JSONHANDLER_H_

#include <ArduinoJson.h>
#include "DHTHandler.h"

String serializa_JSON_string (struct registro_datos datos);
const char* serializa_JSON_char (struct registro_datos datos);

#endif
