#include "ir_tv.h"
#include "ir_codigos_mundo.h"
#include <M5Cardputer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const uint16_t PIN_IR = 44;
IRsend irsend(PIN_IR);

// Marcas muy comunes en España, identificadas por su protocolo (no vienen
// en la base de datos EU de abajo, que son códigos "en crudo" sin marca).
// Van primero porque son las más probables de encontrarte delante.
struct CodigoTv {
    decode_type_t protocolo;
    uint64_t codigo;
    uint16_t bits;
    const char* nombre;
};

static const CodigoTv CODIGOS_PROBABLES[] = {
    { SAMSUNG, 0xE0E040BF, 32, "Samsung" },
    { NEC,     0x20DF10EF, 32, "LG" },
    { SONY,    0xA90,      12, "Sony" },
};
static const int NUM_PROBABLES = sizeof(CODIGOS_PROBABLES) / sizeof(CODIGOS_PROBABLES[0]);

static bool disparando = false;
static int indiceActual = 0;

static uint8_t bitsleft_r = 0;
static uint8_t bits_r = 0;
static const uint8_t* code_ptr = nullptr;

// Lee 'cuantos' bits (2 o 3, segun el codigo) del flujo empaquetado.
static uint8_t leerBits(uint8_t cuantos) {
    uint8_t tmp = 0;
    for (uint8_t i = 0; i < cuantos; i++) {
        if (bitsleft_r == 0) {
            bits_r = *code_ptr++;
            bitsleft_r = 8;
        }
        bitsleft_r--;
        tmp |= (((bits_r >> bitsleft_r) & 1) << (cuantos - 1 - i));
    }
    return tmp;
}

// Descomprime un codigo del TV-B-Gone (tabla de pares on/off + flujo de
// bits que indexa esa tabla) a un buffer plano y lo envia con sendRaw().
static void enviarCodigo(const IrCode* codigo) {
    static uint16_t buffer[300];

    bitsleft_r = 0;
    code_ptr = codigo->codes;

    uint16_t n = 0;
    for (uint8_t k = 0; k < codigo->numpairs; k++) {
        uint16_t indice = leerBits(codigo->bitcompression) * 2;
        buffer[n++] = codigo->times[indice] * 10;      // on, en us (estaba en decenas de us)
        buffer[n++] = codigo->times[indice + 1] * 10;   // off, en us
    }

    irsend.sendRaw(buffer, n, codigo->frecuencia_hz);
}

void irTvSetup() {
    irsend.begin();
    disparando = false;

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.println("TV-B-Gone");
    M5Cardputer.Display.printf("%d codigos (EU)\n", NUM_PROBABLES + num_EUcodes);
    M5Cardputer.Display.println("enter = disparar");
}

void irTvLoop(bool cambioTeclado) {
    int total = NUM_PROBABLES + num_EUcodes;

    if (!disparando) {
        if (cambioTeclado && M5Cardputer.Keyboard.isPressed() && M5Cardputer.Keyboard.keysState().enter) {
            disparando = true;
            indiceActual = 0;
        }
        return;
    }

    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_CYAN);
    M5Cardputer.Display.printf("%d/%d\n", indiceActual + 1, total);
    M5Cardputer.Display.setTextColor(TFT_WHITE);
    M5Cardputer.Display.println("del = cancelar");

    if (indiceActual < NUM_PROBABLES) {
        const CodigoTv& c = CODIGOS_PROBABLES[indiceActual];
        irsend.send(c.protocolo, c.codigo, c.bits);
    } else {
        enviarCodigo(EUpowerCodes[indiceActual - NUM_PROBABLES]);
    }
    delay(200);
    indiceActual++;

    if (indiceActual >= total) {
        disparando = false;
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setCursor(0, 0);
        M5Cardputer.Display.setTextColor(TFT_GREEN);
        M5Cardputer.Display.println("Listo.");
        M5Cardputer.Display.println("enter = repetir");
    }
}
