//#include <Arduino.h>

extern volatile int pulsador_estado_int;     // por defecto HIGH (PULLUP). Cuando se pulsa se pone a LOW
extern volatile int pulsador_evento;         // se activa cuando sucede una interrupcion


extern volatile unsigned long pulsador_ultima_int;
extern volatile unsigned long pulsador_btn_baja;

void setup_pulsador_int();
/*
unsigned long getDuracionPulsacion();
bool getEvento();
void setEventoLOW();
bool getEstadoInt();
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
