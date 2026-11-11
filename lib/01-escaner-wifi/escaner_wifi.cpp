#include "escaner_wifi.h"
#include <M5Cardputer.h>
#include <WiFi.h>

const char* nombreCifrado(wifi_auth_mode_t modo) {
    switch (modo) {
        case WIFI_AUTH_OPEN:         return "ABIERTA";
        case WIFI_AUTH_WEP:          return "WEP";
        case WIFI_AUTH_WPA_PSK:      return "WPA";
        case WIFI_AUTH_WPA2_PSK:     return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA3_PSK:     return "WPA3";
        default:                     return "?";
    }
}

uint16_t colorPorRssi(int32_t rssi) {
    if (rssi >= -60) return TFT_GREEN;
    if (rssi >= -75) return TFT_ORANGE;
    return TFT_RED;
}

static int ultimoConteo = 0;
static int desplazamiento = 0;
static const int REDES_POR_PAGINA = 3;
static int seleccionRed = 0;

void dibujarPagina() {
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);

    if (ultimoConteo <= 0) {
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.println("Sin redes.");
        return;
    }

    M5Cardputer.Display.setTextColor(TFT_CYAN);
    M5Cardputer.Display.printf("%d redes\n", ultimoConteo);

    int fin = desplazamiento + REDES_POR_PAGINA;
    if (fin > ultimoConteo) fin = ultimoConteo;

    for (int i = desplazamiento; i < fin; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) ssid = "(oculta)";
        if (ssid.length() > 12) ssid = ssid.substring(0, 11) + "~";

        int32_t rssi = WiFi.RSSI(i);
        int canal = WiFi.channel(i);

        if (i == seleccionRed) {
            M5Cardputer.Display.setTextColor(TFT_BLACK, colorPorRssi(rssi));
        } else {
            M5Cardputer.Display.setTextColor(colorPorRssi(rssi));
        }

        M5Cardputer.Display.printf("%-12s c%-2d %4ld %s\n",
            ssid.c_str(), canal, (long)rssi, nombreCifrado(WiFi.encryptionType(i)));
    }
}

void escanear() {
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.println("Escaneando...");

    WiFi.scanDelete();
    ultimoConteo = WiFi.scanNetworks();
    desplazamiento = 0;
    seleccionRed = 0;

    dibujarPagina();
}

void escanerWifiSetup() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    escanear();
}

void escanerWifiLoop(bool cambioTeclado) {
    if (cambioTeclado && M5Cardputer.Keyboard.isPressed()) {
        if (M5Cardputer.Keyboard.isKeyPressed(';') && seleccionRed > 0) {
            seleccionRed--;
            if (seleccionRed < desplazamiento) {
                desplazamiento = seleccionRed;
            }
            dibujarPagina();
        }

        if (M5Cardputer.Keyboard.isKeyPressed('.') && seleccionRed < ultimoConteo - 1) {
            seleccionRed++;
            if (seleccionRed >= desplazamiento + REDES_POR_PAGINA) {
                desplazamiento = seleccionRed - REDES_POR_PAGINA + 1;
            }
            dibujarPagina();
        }
    }
}
