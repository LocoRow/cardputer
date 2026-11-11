#include "mqtt_cliente.h"
#include "entrada_texto.h"
#include "conexion_wifi.h"
#include <M5Cardputer.h>
#include <WiFi.h>
#include <PubSubClient.h>

static const char* BROKER_IP = "192.168.0.114";
static const int BROKER_PUERTO = 1883;
static const char* TOPIC_TEST = "cardputer/test";

static String ultimoMensaje = "(nada aun)";
static String mensajeMostrado = "";

WiFiClient clienteWifi;
PubSubClient mqtt(clienteWifi);

void recibirMensaje(char* topic, byte* payload, unsigned int length);

void recibirMensaje(char* topic, byte* payload, unsigned int length) {
    String texto = "";
    for (unsigned int i = 0; i < length; i++) {
        texto += (char)payload[i];
    }
    ultimoMensaje = texto;
}

void mqttSetup() {
    if (!conectarWifi()) {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setCursor(0, 0);
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.println("No se pudo conectar.\ndel = volver");
        return;
    }

    mqtt.setServer(BROKER_IP, BROKER_PUERTO);

    mqtt.setCallback(recibirMensaje);

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.println("Conectando MQTT...");

    String idCliente = "cardputer-" + String(random(0xffff), HEX);
    bool conectado = mqtt.connect(idCliente.c_str());

    if (!conectado) {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setCursor(0, 0);
        M5Cardputer.Display.setTextColor(TFT_RED);
        M5Cardputer.Display.printf("MQTT fallo, codigo %d\ndel = volver", mqtt.state());
        return;
    }

    mqtt.subscribe(TOPIC_TEST);

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_GREEN);
    M5Cardputer.Display.println("MQTT conectado.");
    M5Cardputer.Display.println("Esperando mensajes...");

}

// Si la conexion se cae (p.ej. por quedarnos un rato escribiendo un mensaje
// y perder el keepalive), lo reintenta cada 2s en vez de quedarse muerto.
void reconectarMqttSiHaceFalta() {
    static uint32_t ultimoIntento = 0;
    if (millis() - ultimoIntento < 2000) return;
    ultimoIntento = millis();

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_RED);
    M5Cardputer.Display.println("MQTT desconectado,\nreconectando...");

    String idCliente = "cardputer-" + String(random(0xffff), HEX);
    if (mqtt.connect(idCliente.c_str())) {
        mqtt.subscribe(TOPIC_TEST);
        mensajeMostrado = "";  // fuerza a redibujar "MQTT conectado" otra vez
    }
}

void mqttLoop(bool cambioTeclado) {
    if (!mqtt.connected()) {
        reconectarMqttSiHaceFalta();
        return;
    }

    mqtt.loop();

    if (ultimoMensaje != mensajeMostrado) {
        mensajeMostrado = ultimoMensaje;

        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setCursor(0, 0);
        M5Cardputer.Display.setTextColor(TFT_GREEN);
        M5Cardputer.Display.println("MQTT conectado.");
        M5Cardputer.Display.setTextColor(TFT_WHITE);
        M5Cardputer.Display.println("Ultimo mensaje:");
        M5Cardputer.Display.println(mensajeMostrado);
        M5Cardputer.Display.println("\nenter = enviar");
    }

    if (cambioTeclado && M5Cardputer.Keyboard.isPressed() && M5Cardputer.Keyboard.keysState().enter) {
        String texto = pedirTexto("Mensaje:", false);
        mqtt.publish(TOPIC_TEST, texto.c_str());
    }
}
