#pragma once
#include <Arduino.h>

// Conecta a una red ya elegida: usa la contraseña guardada si coincide con
// esta red, y si no la pide. Guarda la contraseña si consigue conectar.
// Devuelve true si consigue conectar.
bool conectarA(const String& ssid, bool abierta);

// Escanea, pinta la lista y deja elegir una red (;/. para moverse, enter
// para elegir), y luego llama a conectarA con lo elegido.
// Devuelve true si consigue conectar.
bool conectarWifi();
