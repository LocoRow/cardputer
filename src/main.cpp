#include "escaner_wifi.h"
#include "monitor_red.h"
#include "mqtt_cliente.h"
#include "ir_tv.h"
#include "distancia_rssi.h"
#include <M5Cardputer.h>
#include "animacion_inicio.h"

struct Proyecto {
    const char* nombre;
    void (*setup)();
    void (*loop)(bool);
};

Proyecto proyectos[] = {
    { "Escaner WiFi", escanerWifiSetup, escanerWifiLoop },
    { "Monitor Red", monitorRedSetup, monitorRedLoop },
    { "Cliente MQTT", mqttSetup, mqttLoop },
    { "TV-B-Gone", irTvSetup, irTvLoop },
    { "Distancia RSSI", distanciaRssiSetup, distanciaRssiLoop },
};

const int NUM_PROYECTOS = sizeof(proyectos) / sizeof(proyectos[0]);

enum Pantalla { MENU, PROYECTO };
Pantalla pantallaActual = MENU;

int seleccion = 0;

void dibujarMenu() {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(0, 0);

    M5Cardputer.Display.setTextColor(TFT_CYAN, TFT_BLACK);
    M5Cardputer.Display.println("Menu:");

    for (int i = 0; i < NUM_PROYECTOS; i++) {
        if (i == seleccion) {
            M5Cardputer.Display.setTextColor(TFT_BLACK, TFT_GREEN);
        } else {
            M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        M5Cardputer.Display.println(proyectos[i].nombre);
    }
}

void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);

    M5Cardputer.Display.setRotation(0);

    dibujarAnimacionInicio();  // se queda aquí, en bucle, hasta que pulses Enter

    dibujarMenu();
}

void loop() {
    M5Cardputer.update();

    bool cambioTeclado = M5Cardputer.Keyboard.isChange();
    bool pulsado = M5Cardputer.Keyboard.isPressed();
    bool pulsoEnter = cambioTeclado && pulsado && M5Cardputer.Keyboard.keysState().enter;

    switch (pantallaActual) {
        case MENU:
            if (cambioTeclado && pulsado) {
                if (M5Cardputer.Keyboard.isKeyPressed(';') && seleccion > 0) {
                    seleccion--;
                    dibujarMenu();
                }
                if (M5Cardputer.Keyboard.isKeyPressed('.') && seleccion < NUM_PROYECTOS - 1) {
                    seleccion++;
                    dibujarMenu();
                }
                if (pulsoEnter) {
                    pantallaActual = PROYECTO;
                    proyectos[seleccion].setup();
                }
            }
            break;

                case PROYECTO:
            if (cambioTeclado && pulsado && M5Cardputer.Keyboard.keysState().del) {
                pantallaActual = MENU;
                dibujarMenu();
            } else {
                proyectos[seleccion].loop(cambioTeclado);
            }
        break;
    }
}