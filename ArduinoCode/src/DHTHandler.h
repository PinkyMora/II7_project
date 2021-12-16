#ifndef DHTHANDLER_H_
#define DHTHANDLER_H_

#include "DHTesp.h"


struct registro_datos {
  float temperatura;
  float humedad;
  unsigned long tiempo;
};

class OurDHT
{
  public:
    OurDHT(int pin);
    void read(struct registro_datos& datos);
  private:
    DHTesp _dht;
    unsigned long _time;
};




#endif
