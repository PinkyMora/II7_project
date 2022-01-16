// -------------------------------------------------------------------
// Ejemplo de contol de un pulsador mediante interrupciones
// -------------------------------------------------------------------

#include "pulsador_int.h"
/*
extern volatile int estado_int;     // por defecto HIGH (PULLUP). Cuando se pulsa se pone a LOW
extern volatile int evento;        // se activa cuando sucede una interrupcion

extern volatile unsigned long ultima_int;
extern volatile unsigned long btn_baja;
*/
/*------------------ SETUP --------------------*/
void setup() {
  Serial.begin(115200);
  setup_pulsador_int();
}

/*------------------- LOOP --------------------*/
void loop() {
  if(pulsador_evento==HIGH)
  {
    pulsador_evento = LOW;
    if (pulsador_estado_int == LOW)
    {
      Serial.printf("Se detecto una pulsacion en %d ms, \n", pulsador_btn_baja);
    }
    else
    {
      Serial.printf("Termino la pulsacion, duracion de %d ms\n", pulsador_ultima_int-pulsador_btn_baja);
    }
  }
}
