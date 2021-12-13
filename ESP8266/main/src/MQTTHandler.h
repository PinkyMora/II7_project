#ifndef MQTTHANDLER_H_
#define MQTTHANDLER_H_

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MQTTHandler
{
	public:
		MQTTHandler(WiFiClient client, const char* server);
		MQTTHandler(WiFiClient client, const char* server, const char* user, const char* pass);
		int sendData(String datos);
		void loop();
	private:
		PubSubClient* _client;
		const char* _user;
		const char* _pass;
		char _ID_PLACA[16];
		char _topic_PUB[256];
		char _topic_SUB[256];
		void reconnect();

};

#endif
