#include "entrada_texto.h"
#include <M5Cardputer.h>

void dibujarEntradaTexto(const char* titulo, const String& texto, bool ocultar) {
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setCursor(0, 0);
    M5Cardputer.Display.setTextColor(TFT_CYAN);
    M5Cardputer.Display.println(titulo);

    M5Cardputer.Display.setTextColor(TFT_WHITE);
    if (ocultar) {
        for (size_t i = 0; i < texto.length(); i++) {
            M5Cardputer.Display.print('*');
        }
        M5Cardputer.Display.println();
    } else {
        M5Cardputer.Display.println(texto);
    }
}

String pedirTexto(const char* titulo, bool ocultar) {
    String texto = "";
    dibujarEntradaTexto(titulo, texto, ocultar);

    while (true) {
        M5Cardputer.update();

        if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
            auto estado = M5Cardputer.Keyboard.keysState();

            if (estado.enter) {
                break;
            }

            if (estado.del && texto.length() > 0) {
                texto.remove(texto.length() - 1);
            }

            for (char c : estado.word) {
                texto += c;
            }

            dibujarEntradaTexto(titulo, texto, ocultar);
        }
    }

    return texto;
}