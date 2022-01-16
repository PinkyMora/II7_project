# Uso de funciones y variables para tratamiento de pulsaciones

Para utilizar las funciones y variables de los archivos de esta carpeta, añadir al directorio proyecto (o a proyecto/src) tanto **pulsador_int.cpp** como **pulsador_int.h**. Añadir al fichero principal del proyecto .ino la línea:
```
#include "pulsador_int.h" 
```
De esta forma se podrán usar las siguientes funciones y variables:
```
void setup_pulsador_int();

volatile int pulsador_estado_int;
volatile int pulsador_evento;
volatile unsigned long pulsador_ultima_int;
volatile unsigned long pulsador_btn_baja;
```
No es necesario definir ni declarar ninguna de estas variables.

Se incluye el ejemplo de uso en **pulsador_int.ino**.
```
#include "pulsador_int.h"

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
```
