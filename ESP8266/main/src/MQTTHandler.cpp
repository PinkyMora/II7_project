#include "MQTTHandler.h"


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


MQTTHandler::MQTTHandler(WiFiClient client, const char* server)
{
  _client = new PubSubClient(client);
  _client->setServer(server, 1883);
  _client->setCallback(callback);
  _client->setBufferSize(512); // para poder enviar mensajes de hasta X bytes
  _user = NULL;
  _pass = NULL;
  sprintf(_ID_PLACA, "ESP_%d", ESP.getChipId());
  sprintf(_topic_PUB, "infind/%s/pub", _ID_PLACA);
  sprintf(_topic_SUB, "infind/%s/sub", _ID_PLACA);

}

MQTTHandler::MQTTHandler(WiFiClient client, const char* server, const char* user, const char* pass)
{
  _client = new PubSubClient(client);
  _client->setServer(server, 1883);
  _client->setCallback(callback);
  _client->setBufferSize(512); // para poder enviar mensajes de hasta X bytes
  _user = user;
  _pass = pass;
  sprintf(_ID_PLACA, "ESP_%d", ESP.getChipId());
  sprintf(_topic_PUB, "infind/%s/pub", _ID_PLACA);
  sprintf(_topic_SUB, "infind/%s/sub", _ID_PLACA);
}


int MQTTHandler::sendData(String datos)
{
	return _client->publish(_topic_PUB, datos.c_str());
}

void MQTTHandler::loop()
{
	if (!_client->connected()) {
    	reconnect();
 	}
 	_client->loop();
}

void MQTTHandler::reconnect() {
  // Loop until we're reconnected
  while (!_client->connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (_client->connect(_ID_PLACA, _user, _pass)) {
      Serial.println("connected");
      // ... and resubscribe
      _client->subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(_client->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}