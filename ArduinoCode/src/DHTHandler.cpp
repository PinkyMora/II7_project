#include "DHTHandler.h"

OurDHT::OurDHT(int pin)
{
  _dht.setup(pin, DHTesp::DHT11);
  _time = millis();
}

void OurDHT::read(struct registro_datos& datos)
{
  while(_time + _dht.getMinimumSamplingPeriod() < millis()){}
  _time = millis();
  datos.temperatura=_dht.getTemperature();
  datos.humedad=_dht.getHumidity();
  datos.tiempo=millis();
}
