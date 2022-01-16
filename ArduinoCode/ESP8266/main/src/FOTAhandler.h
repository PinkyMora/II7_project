#ifndef FOTAHANDLER_H_
#define FOTAHANDLER_H_

// Descomentar para ESP8266
//#include <ESP8266WiFi.h>
//#include <ESP8266httpUpdate.h>

// Descomentar para ESP32
//#include <WiFi.h>
//#include <ESP32httpUpdate.h>

class FOTAHandler
{
	public:
		void intenta_OTA();
	private:
		void final_OTA();
        void inicio_OTA();
        void error_OTA(int e);
        void progreso_OTA(int x, int todo);
};

#endif