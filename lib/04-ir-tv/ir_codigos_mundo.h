#pragma once
#include <Arduino.h>

// Base de datos original del TV-B-Gone (Mitch Altman / Limor Fried,
// portado a Arduino por Ken Shirriff, licencia CC 2.5 Atribucion+CompartirIgual):
// https://github.com/shirriff/Arduino-TV-B-Gone
//
// Cada codigo guarda una tabla pequeña de pares on/off (en decenas de
// microsegundos) y un flujo de bits empaquetado que indexa esa tabla,
// para no repetir los mismos tiempos una y otra vez.
//
// En el original, el primer campo era el valor de registro de un timer
// de AVR (de ahi "freq_to_timerval"). Aqui no hay ese hardware, asi que
// redefinimos la macro para que guarde la frecuencia real en Hz tal cual.
#define NUM_ELEM(x) (sizeof(x) / sizeof(*(x)))
#define freq_to_timerval(hz) (hz)

struct IrCode {
    uint32_t frecuencia_hz;
    uint8_t numpairs;
    uint8_t bitcompression;
    uint16_t const *times;
    uint8_t const *codes;
};

extern const IrCode* const NApowerCodes[] PROGMEM;
extern const IrCode* const EUpowerCodes[] PROGMEM;
extern uint8_t num_NAcodes;
extern uint8_t num_EUcodes;
