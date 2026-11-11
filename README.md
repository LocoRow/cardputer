# Cardputer — proyectos de aprendizaje en redes y ciberseguridad

Firmware único para el M5Stack Cardputer ADV (ESP32-S3), con un menú en
pantalla para elegir entre varias herramientas. Cada una es un ejercicio de
aprendizaje independiente, construido paso a paso.

## Herramientas

1. **Escáner WiFi** — lista las redes cercanas con su señal, canal y tipo de
   cifrado.
2. **Monitor de red** — tras conectarte a una red, barre la subred,
   identifica los hosts activos (ping + tabla ARP + consulta a
   macvendors.com) y comprueba qué puertos comunes tienen abiertos.
3. **Cliente MQTT** — se conecta a un broker Mosquitto, se suscribe a un
   topic y deja publicar mensajes desde el teclado. Recuerda la última red y
   contraseña usadas.
4. **TV-B-Gone** — emisor IR que recorre la base de datos original del
   proyecto TV-B-Gone para intentar apagar cualquier televisor cercano.
5. **Distancia por RSSI** — modo promiscuo WiFi: estima a qué distancia
   aproximada está un router según la potencia de su señal.

## Hardware

- M5Stack Cardputer ADV (ESP32-S3 / Stamp-S3A).

## Cómo compilar

Con [PlatformIO](https://platformio.org/):

```bash
pio run -t upload --upload-port <puerto>
```

## Créditos

- La base de datos de códigos IR de "TV-B-Gone" viene de
  [shirriff/Arduino-TV-B-Gone](https://github.com/shirriff/Arduino-TV-B-Gone)
  (Mitch Altman + Limor Fried 2009, portado a Arduino por Ken Shirriff),
  licencia Creative Commons Atribución + CompartirIgual 2.5.
