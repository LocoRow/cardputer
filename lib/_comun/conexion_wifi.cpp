#include "conexion_wifi.h"
#include "credenciales_wifi.h"
#include "entrada_texto.h"
#include <M5Cardputer.h>
#include <WiFi.h>

static const int REDES_POR_PAGINA = 3;
static int numRedesEncontradas = 0;
static int seleccionRed = 0;
static int desplazamientoRedes = 0;

static void dibujarListaRedes() {
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

// Bloquea hasta que elijas una red: escanea, pinta la lista y deja navegar
// con ;/. y confirmar con enter.
static String elegirRed(bool* esAbierta) {
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
                String ssid = WiFi.SSID(seleccionRed);
                *esAbierta = (WiFi.encryptionType(seleccionRed) == WIFI_AUTH_OPEN);
                WiFi.scanDelete();
                return ssid;
            }
        }
        delay(30);
    }
}

bool conectarA(const String& ssid, bool abierta) {
    String password = "";
    if (!abierta) {
        password = contrasenaGuardada(ssid);
        if (password.length() == 0) {
            password = pedirTexto("Contraseña:", true);
        }
    }

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.println("Conectando WiFi...");

    WiFi.mode(WIFI_STA);
    if (abierta) {
        WiFi.begin(ssid.c_str());
    } else {
        WiFi.begin(ssid.c_str(), password.c_str());
    }

    uint32_t inicio = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
        delay(200);
    }

    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    if (!abierta) {
        guardarCredenciales(ssid, password);
    }
    return true;
}

bool conectarWifi() {
    bool abierta = false;
    String ssid = elegirRed(&abierta);
    return conectarA(ssid, abierta);
}
