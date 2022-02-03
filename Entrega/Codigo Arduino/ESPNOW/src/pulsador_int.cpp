// -------------------------------------------------------------------
// Contol de un pulsador mediante interrupciones
// -------------------------------------------------------------------

#include <Arduino.h>

int boton_flash=0;       // GPIO0 = boton flash


volatile int pulsador_estado_int = HIGH;     // por defecto HIGH (PULLUP). Cuando se pulsa se pone a LOW
volatile int pulsador_evento = LOW;        // se activa cuando sucede una interrupcion


volatile unsigned long pulsador_ultima_int = 0;
volatile unsigned long pulsador_btn_baja = 0;

unsigned long ahora = 0;

/*------------------  RTI  --------------------*/
// Rutina de Tratamiento de la Interrupcion (RTI)
ICACHE_RAM_ATTR void RTI() {
  int lectura=digitalRead(boton_flash);
  ahora= millis();
  
  // descomentar para eliminar rebotes
  if(lectura==pulsador_estado_int || ahora-pulsador_ultima_int<50) return; // filtro rebotes 50ms

  pulsador_evento = HIGH;
  
  if(lectura==LOW)
  { 
   pulsador_estado_int=LOW;
   pulsador_btn_baja = ahora;
  }
  else
  {
   pulsador_estado_int=HIGH;
  }
  pulsador_ultima_int = ahora;
}

/*------------------ SETUP --------------------*/
void setup_pulsador_int() {
  pinMode(boton_flash, INPUT_PULLUP);
  // descomentar para activar interrupción
  attachInterrupt(digitalPinToInterrupt(boton_flash), RTI, CHANGE);
}

/*

unsigned long getDuracionPulsacion(){
  return pulsador_ultima_int-pulsador_btn_baja;
}

bool getEvento(){
  return pulsador_evento;
}

void setEventoLOW(){
  pulsador_evento = LOW;
}

bool getEstadoInt(){
  return pulsador_estado_int;
}

*/

/*------------------- LOOP --------------------*/
/*  En el loop del fichero principal .ino del programa, utilizar la siguiente
 *  estructura para tratar las pulsaciones y el tiempo de pulsación.
void loop() {
  if(evento==HIGH)
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
*/
