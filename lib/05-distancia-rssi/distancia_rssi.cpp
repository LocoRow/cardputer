#include "distancia_rssi.h"
#include <M5Cardputer.h>
#include <WiFi.h>
#include "esp_wifi.h"

static uint8_t bssidObjetivo[6];
static int canalObjetivo = 0;
static int ultimoRssi = 0;

static int numRedesEncontradas = 0;
static int seleccionRed = 0;
static int desplazamientoRedes = 0;
static const int REDES_POR_PAGINA = 3;

void dibujarListaRedes() {
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);

    if (numRedesEncontradas <= 0) {
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.println("Sin redes.");
        return;
    }

    M5Cardputer.Display.setTextColor(TFT_CYAN);
    M5Cardputer.Display.printf("%d redes\n", numRedesEncontradas);

    int fin = desplazamientoRedes + REDES_POR_PAGINA;
    if (fin > numRedesEncontradas) fin = numRedesEncontradas;

    for (int i = desplazamientoRedes; i < fin; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) ssid = "(oculta)";
        if (ssid.length() > 14) ssid = ssid.substring(0, 13) + "~";

        if (i == seleccionRed) {
            M5Cardputer.Display.setTextColor(TFT_BLACK, TFT_GREEN);
        } else {
            M5Cardputer.Display.setTextColor(TFT_WHITE);
        }
        M5Cardputer.Display.println(ssid);
    }
}

void elegirRedObjetivo() {
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.println("Escaneando...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.scanDelete();
    numRedesEncontradas = WiFi.scanNetworks();
    seleccionRed = 0;
    desplazamientoRedes = 0;

    dibujarListaRedes();

    while (true) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            auto estado = M5Cardputer.Keyboard.keysState();

            if (M5Cardputer.Keyboard.isKeyPressed(';') && seleccionRed > 0) {
                seleccionRed--;
                if (seleccionRed < desplazamientoRedes) desplazamientoRedes = seleccionRed;
                dibujarListaRedes();
            }
            if (M5Cardputer.Keyboard.isKeyPressed('.') && seleccionRed < numRedesEncontradas - 1) {
                seleccionRed++;
                if (seleccionRed >= desplazamientoRedes + REDES_POR_PAGINA) {
                    desplazamientoRedes = seleccionRed - REDES_POR_PAGINA + 1;
                }
                dibujarListaRedes();
            }
            if (estado.enter && numRedesEncontradas > 0) {
                memcpy(bssidObjetivo, WiFi.BSSID(seleccionRed), 6);
                canalObjetivo = WiFi.channel(seleccionRed);
                WiFi.scanDelete();
                return;
            }
        }
        delay(30);
    }
}

void IRAM_ATTR callbackPromiscuo(void *buf, wifi_promiscuous_pkt_type_t tipo) {
    wifi_promiscuous_pkt_t *paquete = (wifi_promiscuous_pkt_t *)buf;
    uint8_t *datos = paquete->payload;
    uint8_t *origen = datos + 10;  // bytes 10-15 de la cabecera 802.11

    if (memcmp(origen, bssidObjetivo, 6) == 0) {
        ultimoRssi = paquete->rx_ctrl.rssi;
    }
}

void distanciaRssiSetup() {
    elegirRedObjetivo();
    ultimoRssi = 0;

    // El WiFi ya queda en modo STA desconectado tras elegirRedObjetivo():
    // justo lo que hace falta. Poner aqui WIFI_MODE_NULL para el driver
    // entero en Arduino-ESP32 y el modo promiscuo deja de recibir nada.
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&callbackPromiscuo);
    esp_wifi_set_channel(canalObjetivo, WIFI_SECOND_CHAN_NONE);

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.printf("Canal %d\nEscuchando...\n", canalObjetivo);
}

void distanciaRssiLoop(bool cambioTeclado) {
    static uint32_t ultimaActualizacion = 0;
    static const float RSSI_A_1M = -40.0;
    static const float FACTOR_ENTORNO = 3.0;

    if (millis() - ultimaActualizacion < 300) return;
    ultimaActualizacion = millis();

    float distancia = pow(10.0, (RSSI_A_1M - ultimoRssi) / (10.0 * FACTOR_ENTORNO));

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.printf("RSSI: %d dBm\n", ultimoRssi);
    M5Cardputer.Display.setTextColor(TFT_CYAN);
    M5Cardputer.Display.printf("~%.1f m\n", distancia);
}
