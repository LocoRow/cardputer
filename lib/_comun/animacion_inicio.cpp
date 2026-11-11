#include "animacion_inicio.h"
#include <M5Cardputer.h>

bool enterPulsado() {
    M5Cardputer.update();
    return M5Cardputer.Keyboard.isChange()
           && M5Cardputer.Keyboard.isPressed()
           && M5Cardputer.Keyboard.keysState().enter;
}

void dibujarAnimacionInicio() {
    int anchoPantalla = M5Cardputer.Display.width();
    int altoPantalla = M5Cardputer.Display.height();

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextSize(3);
    M5Cardputer.Display.setTextColor(TFT_BLUE);
    int anchoTexto = M5Cardputer.Display.textWidth("Loco-Shark");
    M5Cardputer.Display.setCursor((anchoPantalla - anchoTexto) / 2, (altoPantalla - 24) / 2);
    M5Cardputer.Display.println("Loco-Shark");

    while (!enterPulsado()) {
        delay(50);
    }
}
