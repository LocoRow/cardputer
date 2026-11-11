#include "monitor_red.h"
#include "conexion_wifi.h"
#include <M5Cardputer.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include <HTTPClient.h>

static const int HOSTS_POR_PAGINA = 2;
static int desplazamientoResultados = 0;

static IPAddress ipBase;
static int siguienteHost = 1;
static int numEncontrados = 0;
static int encontrados[50];
static bool barriendoActivo = false;

static bool escaneandoPuertos = false;
static int hostActual = 0;
static int puertoActual = 0;

static const int PUERTOS[] = {80, 443, 22, 554, 9100};
static const int NUM_PUERTOS = 5;
static String etiquetas[50];

static String fabricante[50];
static String macCompleta[50];
static int codigoOnline[50];

void mensaje(const String& texto, uint16_t color) {
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(color);
    M5Cardputer.Display.println(texto);
}

struct Fabricante {
    uint8_t b0, b1, b2;
    const char* nombre;
};

static const Fabricante FABRICANTES[] = {
    // Bloques "de catalogo" (comprobados en macvendors.com, pero puede que
    // no coincidan con el bloque concreto que use cada aparato: las marcas
    // grandes tienen muchos).
    {0xB8, 0x27, 0xEB, "Raspberry Pi"},
    {0x24, 0x0A, 0xC4, "Espressif (ESP32/8266)"},
    {0xF0, 0x18, 0x98, "Apple"},
    {0x50, 0xC7, 0xBF, "TP-Link"},
    {0x8C, 0xF5, 0xA3, "Samsung"},
    {0x44, 0x65, 0x0D, "Amazon"},
    {0xF4, 0xF5, 0xD8, "Google"},
    {0x64, 0x09, 0x80, "Xiaomi"},
    // Bloques de VERDAD, vistos en la ARP de esta casa (vale la pena tenerlos,
    // son los que de verdad vas a encontrar en tu red):
    {0x30, 0x68, 0x93, "TP-Link (router de casa)"},
    {0x3C, 0x0F, 0x02, "Espressif (ESP32/8266)"},
    {0x2C, 0xCF, 0x67, "Raspberry Pi"},
    {0x54, 0x44, 0xA3, "Samsung"},
    {0xF0, 0xEF, 0x86, "Google"},
    {0xC8, 0x58, 0xB3, "Intel (PC/portatil)"},
};

static const int NUM_FABRICANTES = sizeof(FABRICANTES) / sizeof(FABRICANTES[0]);

String buscarFabricante(uint8_t b0, uint8_t b1, uint8_t b2) {
    for (int i = 0; i < NUM_FABRICANTES; i++) {
        if (FABRICANTES[i].b0 == b0 && FABRICANTES[i].b1 == b1 && FABRICANTES[i].b2 == b2) {
            return FABRICANTES[i].nombre;
        }
    }
    return "?";
}

String consultarFabricanteOnline(const String& mac, int indice) {
    HTTPClient http;
    http.begin("http://api.macvendors.com/" + mac);
    http.setTimeout(1500);

    int codigo = http.GET();
    String resultado = "?";
    if (codigo == 200) {
        resultado = http.getString();
    }

    codigoOnline[indice] = codigo;

    Serial.printf("[macvendors] %s -> codigo %d, resultado: %s\n",
                  mac.c_str(), codigo, resultado.c_str());

    http.end();
    return resultado;
}

void monitorRedSetup() {
    if (!conectarWifi()) {
        mensaje("No se pudo conectar.\ndel = volver", TFT_RED);
        return;
    }

    mensaje("Conectado:\n" + WiFi.localIP().toString(), TFT_GREEN);

    IPAddress ip = WiFi.localIP();
    IPAddress mascara = WiFi.subnetMask();
    ipBase = IPAddress(ip[0] & mascara[0], ip[1] & mascara[1], ip[2] & mascara[2], ip[3] & mascara[3]);

    siguienteHost = 1;
    numEncontrados = 0;
    barriendoActivo = true;
    for (int i = 0; i < 50; i++) codigoOnline[i] = -1;
}

void dibujarResultados() {
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);

    if (numEncontrados == 0) {
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.println("Nadie respondio.");
        return;
    }

    M5Cardputer.Display.setTextColor(TFT_CYAN);
    M5Cardputer.Display.printf("%d activos\n", numEncontrados);

    int fin = desplazamientoResultados + HOSTS_POR_PAGINA;
    if (fin > numEncontrados) fin = numEncontrados;
        for (int i = desplazamientoResultados; i < fin; i++) {
        M5Cardputer.Display.setTextColor(TFT_GREEN);
        M5Cardputer.Display.printf("%d.%d.%d.%d\n", ipBase[0], ipBase[1], ipBase[2], encontrados[i]);

        M5Cardputer.Display.setTextColor(TFT_WHITE);
        String textoPuertos = etiquetas[i].length() > 0 ? etiquetas[i] : "S/P";
        M5Cardputer.Display.println("  " + fabricante[i] + " / " + textoPuertos);

        // Linea de depuracion: que vimos realmente para este host.
        M5Cardputer.Display.setTextSize(1);
        M5Cardputer.Display.setTextColor(TFT_YELLOW);
        if (macCompleta[i].length() == 0) {
            M5Cardputer.Display.println("  sin ARP");
        } else {
            String macCorta = macCompleta[i].substring(9);  // ultimos 3 bytes
            String estadoOnline = codigoOnline[i] == -1 ? "sin consultar" : ("http=" + String(codigoOnline[i]));
            M5Cardputer.Display.println("  " + macCorta + " " + estadoOnline);
        }
        M5Cardputer.Display.setTextSize(2);
    }
}

void monitorRedLoop(bool cambioTeclado) {
    if (barriendoActivo) {
        IPAddress objetivo(ipBase[0], ipBase[1], ipBase[2], siguienteHost);

        if (Ping.ping(objetivo, 1) && numEncontrados < 50) {
            ip4_addr_t destino;
            IP4_ADDR(&destino, ipBase[0], ipBase[1], ipBase[2], siguienteHost);

            struct eth_addr *mac;
            const ip4_addr_t *ipRet;
                       LOCK_TCPIP_CORE();
            int indiceArp = etharp_find_addr(NULL, &destino, &mac, &ipRet);
            if (indiceArp >= 0) {
                fabricante[numEncontrados] = buscarFabricante(mac->addr[0], mac->addr[1], mac->addr[2]);

                char macTexto[18];
                snprintf(macTexto, sizeof(macTexto), "%02X:%02X:%02X:%02X:%02X:%02X",
                         mac->addr[0], mac->addr[1], mac->addr[2], mac->addr[3], mac->addr[4], mac->addr[5]);
                macCompleta[numEncontrados] = macTexto;
                Serial.printf("[arp] %d.%d.%d.%d -> %s (tabla local: %s)\n",
                               ipBase[0], ipBase[1], ipBase[2], siguienteHost,
                               macTexto, fabricante[numEncontrados].c_str());
            } else {
                Serial.printf("[arp] %d.%d.%d.%d -> SIN ENTRADA ARP\n",
                               ipBase[0], ipBase[1], ipBase[2], siguienteHost);
                fabricante[numEncontrados] = "?";
            }
            UNLOCK_TCPIP_CORE();

            encontrados[numEncontrados] = siguienteHost;
            numEncontrados++;
        }

        siguienteHost++;

        if (siguienteHost > 254) {
            barriendoActivo = false;
            escaneandoPuertos = true;
            hostActual = 0;
            puertoActual = 0;
        } else {
            mensaje("Barriendo red...\n" + String(siguienteHost) + "/254\nencontrados: " + String(numEncontrados),
                    TFT_WHITE);
        }
        return;
    }

    if (escaneandoPuertos) {
        if (puertoActual == 0 && fabricante[hostActual] == "?" && macCompleta[hostActual].length() > 0) {
            fabricante[hostActual] = consultarFabricanteOnline(macCompleta[hostActual], hostActual);
        }

        IPAddress objetivo(ipBase[0], ipBase[1], ipBase[2], encontrados[hostActual]);
        int puerto = PUERTOS[puertoActual];

        WiFiClient cliente;
        if (cliente.connect(objetivo, puerto, 300)) {
            if (etiquetas[hostActual].length() > 0) etiquetas[hostActual] += ",";
            etiquetas[hostActual] += String(puerto);
            cliente.stop();
        }

        puertoActual++;
        if (puertoActual >= NUM_PUERTOS) {
            puertoActual = 0;
            hostActual++;
        }

        if (hostActual >= numEncontrados) {
            escaneandoPuertos = false;
            desplazamientoResultados = 0;
            dibujarResultados();
        } else {
            mensaje("Identificando...\n" + String(hostActual + 1) + "/" + String(numEncontrados), TFT_WHITE);
        }
        return;
    }

    // Barrido terminado: navegar los resultados con ;/.
    if (cambioTeclado && M5Cardputer.Keyboard.isPressed()) {
        if (M5Cardputer.Keyboard.isKeyPressed(';') && desplazamientoResultados > 0) {
            desplazamientoResultados -= HOSTS_POR_PAGINA;
            if (desplazamientoResultados < 0) desplazamientoResultados = 0;
            dibujarResultados();
        }
        if (M5Cardputer.Keyboard.isKeyPressed('.') && desplazamientoResultados + HOSTS_POR_PAGINA < numEncontrados) {
            desplazamientoResultados += HOSTS_POR_PAGINA;
            dibujarResultados();
        }
    }
}
