// Librerias necesarias:
#include <ESP8266WiFi.h> 
#include "DHTesp.h"
#include "math.h"

extern "C" { // Utiliza librerias definidas en C, por lo que se introducen con la palabra extern
  #include "user_interface.h"
  #include <espnow.h>
}
  
// keep in sync with ESP_NOW sensor struct
struct __attribute__((packed)) SENSOR_DATA { // packed significa que utilizará el menor espacio posible
 String chipID;
 bool ocupado; 
 float temp;
 float humedad;
 int numPlaza;
 bool ok;
} sensorData;

 // Sensor DHT
 DHTesp dht;
 const int pinDHT = 2; // Pin GPI02 (D4) para sensor DHT

 // Variables Sensores de Proximidad
 const int echo = 5; // Pin GPIO5 (D1) para el Echo del PRIMER sensor de proximidad 
 const int trigger = 4; // Pin digital GPIO4(D2) para el Trigger del PRIMER sensor de proximidad
 long t; // tiempo en us que tarda en llegar el echo
 long d; // distancia en cm
 bool estadoSensorProx = false; // Variable que almacenara el estado del sensor
                                 // Estado a 'false' = plaza libre; Estado a 'true' = plaza ocupada
                                 
 // Identificacion de la placa
 String chipID = "PSensores";

 // Identificacion de plaza
 int numPlaza;

 // Variables Utiles para Robustez del programa
 float lastHum = 69;
 float lastTemp = 17;
 float humedad;
 float temp;
 // Variables ESP-NOW
 uint8_t mac_destino[] = {0x4A, 0x3F, 0xDA, 0x0C, 0xB7, 0xCF}; // AP MAC de Destino
 int canal = 6;
 uint8_t *clave = NULL;
 int longitud_clave = 0;
 unsigned long sentStartTime;
 unsigned long ultimo_mensaje=0;

  void cuando_mensaje_enviado(uint8_t *mac_addr, uint8_t status) {
    Serial.print("Estado del mensaje enviado: ");
    Serial.println((status == 0)? "recibido correctamente" : "no recibido");
    Serial.printf("Enviado y confirmado en %4lu microsegundos \n", micros() - sentStartTime);
    Serial.print("Ahora millis()= ");
    Serial.println(millis());
    Serial.println();
  }

void setup() {
  
    Serial.begin(115200); // Se inicializa la comunicacion
    pinMode(trigger, OUTPUT); // Se le da al pin trigger la condicion de salida
    pinMode(echo, INPUT); // Se le da al pin echo la condicion de entrada
    digitalWrite(trigger, LOW); // Se inicializa el trigger a 0

    dht.setup(pinDHT,DHTesp::DHT11); // Conecta sensor DHT al pin digital definido
  

  
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println();
    Serial.println();
    Serial.print("ESP8266 Board MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print(" enviamos a la MAC : ");
    Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
      mac_destino[0],mac_destino[1],mac_destino[2],mac_destino[3],mac_destino[4],mac_destino[5]);
    Serial.printf("Tamaño del mensaje: %d bytes (máximo 250)\n",sizeof(struct SENSOR_DATA));
    // Init ESP-NOW
    if (esp_now_init() != 0) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(cuando_mensaje_enviado);
     
    // Enlazar con destino
    if (esp_now_add_peer(mac_destino, ESP_NOW_ROLE_SLAVE, canal, clave, longitud_clave) != 0)
    {
      Serial.println("Fallo al enlazar con ESP destino");
      return;
    }
    
  }

void loop() {

    unsigned long ahora= millis();
                                                             
    if (ultimo_mensaje==0 || ahora - ultimo_mensaje >= 30000)
    {
       ultimo_mensaje = ahora;
       // Set values to send
  
        // SE GENERA UN PULSO DE 10us
        digitalWrite(trigger, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigger, LOW);
  
        // CÁLCULO DE LA PROXIMIDAD
        t = pulseIn(echo,HIGH);// Se mide el pulso de respuesta del sensor
        d = t/59;
       
        // Como método de escalabilidad se puede hacer uso de los pines a los que se encuentran conectados
        // a los sensores de proximidad
          if (echo == 5 && trigger == 4)
          {
            numPlaza = 1;
          }

        // ESTADO DE LA PLAZA DE PARKING
          if (d <= 10)//Condicion que implica que una plaza esté ocupada por un vehículo
          {
            estadoSensorProx = true;
          }
          else
          {
            estadoSensorProx = false; 
          }

       // SENSOR DE HUMEDAD Y TEMPERATURA DHT
          humedad = dht.getHumidity(); // Se obtienen los valores de la humedad leidos por el sensor DHT
          temp = dht.getTemperature(); // Se obtienen los valores de la temperatura leidos por el sensor

       
          if (isnan(temp) || isnan(humedad)) //Si el sensor comete un error de medida, obtiene valores fuera de ss rangos de funcionamiento 
          {                                  //Vuelve a leer los valores
              humedad = lastHum; // Se obtienen los valores de la humedad leidos por el sensor DHT
              temp = lastTemp; // Se obtienen los valores de la temperatura leidos por el sensor
          }
          else // Guarda los ultimos valores de temperatura y humedad correctos del sensor DHT
          {
              lastHum = humedad;
              lastTemp = temp;
          }
    
       // Se les da valores a los campos del struct que va a ser enviado
         sensorData.chipID = chipID;
         sensorData.ocupado = estadoSensorProx;
         sensorData.ok = true;
         sensorData.temp = temp;
         sensorData.humedad = humedad;
         sensorData.numPlaza = numPlaza;

       // Se comprueba el valor de los campos enviados
         Serial.print("ChipID:");
         Serial.print(sensorData.chipID);
         Serial.print("//Ocupado:");
         Serial.print(sensorData.ocupado);
         Serial.print("//Temperatura:");
         Serial.print(sensorData.temp);
         Serial.print("//Humedad:");
         Serial.print(sensorData.humedad);
         Serial.print("//NumPlaza:");
         Serial.println(sensorData.numPlaza);
      
     // Se envia el mensaje vía ESP-NOW
     sentStartTime=micros();
     uint8_t resultado = esp_now_send(mac_destino, (uint8_t *) &sensorData, sizeof(sensorData));

       
     if (resultado == 0)
     {
      Serial.println("Envío procesado");
     }
      else
      Serial.println("Error al intentar enviar");
  }
   
}
