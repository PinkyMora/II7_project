# Proyecto de Informática Industrial del Grupo 7 de GIERM del curso 2021-2022.

En este proyecto se ha desarrollado un Garaje IoT. El garaje consta de una entrada con barrera, modelada como un servo conectado a una placa ESP8266 con botón, que al pulsarse lanza una petición de imagen a una ESP32-Cam que apunta al vehículo de entrada. Esta imagen se procesa mediante el servicio online [platerecognizer.com](https://platerecognizer.com), que devuelve un objeto que contiene la matrícula que aparece en la imagen, entre otras cosas. Esta matrícula se guarda en una base de datos y además, el sistema da la orden a la entrada de que suba la barrera, y 10 segundos después, de que se baje.

En paralelo a esto, en las plazas de garaje hay otra placa ESP8266 con sensores de distancia que permiten conocer la ocupación de la plaza, y con sensores de temperatura y humedad. Los datos recogidos por esta placa se empaquetan y se envían por ESP-Now a otra placa ESP8266 que actúa como pasarela con el servicio de manejo de mensajes MQTT. Los datos de estos sensores se guardan en la base de datos.

Todo el sistema de paso de mensajes se ha implementado con MQTT y ESP-Now, y el control central del sistema se hace mediante un sistema SCADA implementado con un servidor con NodeRED. Este servidor se encarga también del almacenado de datos en la base de datos noSQL MongoDB, y de la implementación de la interfaz mediante un dashboard accesible a través de un navegador. También se encarga de controlar un bot de Telegram (@Proyecto_II7_bot) que actúa como segunda interfaz, permitiendo interactuar con él directamente. También se ha creado un [canal de difusión](https://t.me/ParkingII7) en el que se envían notificaciones del funcionamiento del sistema. 

---

## Descripción del repositorio
- Codigo Arduino: contiene los proyectos arduino para compilar y programar en las placas usadas en el proyecto.
- Flows NodeRED: contiene los ficheros .json correspondientes a los flujos utilizados en NodeRED. También contiene una carpeta con las imágenes de la barrera que se utilizan en el flujo DASHBOARD.
- Documentacion y manuales: contiene el pdf final con la memoria del proyecto, con y sin el manual anexado al final, así como el manual de usuario aparte. También contiene una carpeta con las imágenes de las capturas del dashboard utilizadas en el manual de usuario, otra con los diagramas de flujo de los programas de las placas, y otra con los esquemáticos del hardware.
- Entrega: contiene la carpeta de proyecto comprimida en formato .zip entregada en el campus virtual.

---

## Listado de topics:
```
II7/ESP32/conexion
II7/ESP32/config
II7/ESP32/FOTA
II7/SensoresPlazas/conexion
II7/SensoresPlazas/config
II7/SensoresPlazas/FOTA
II7/SensoresPlazas/EstadoPlazas
II7/Entrada/conexion
II7/Entrada/config
II7/Entrada/FOTA
II7/Entrada/Pulsador
II7/Entrada/BarreraEstado
II7/Entrada/BarreraCMD
```
---
## Colecciones de la base de datos II7
- registro_matriculas_entrada
- registro_estado_plazas
- registro_conexion
- registro_barrera_entrada
