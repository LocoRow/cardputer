#pragma once
#include <Arduino.h>

// Si hay una contraseña guardada para este ssid, la devuelve; si no, "".
String contrasenaGuardada(const String& ssid);

// Guarda ssid+contraseña como la última red usada (sobrescribe la anterior).
void guardarCredenciales(const String& ssid, const String& password);
