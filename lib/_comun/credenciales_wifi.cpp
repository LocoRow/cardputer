#include "credenciales_wifi.h"
#include <Preferences.h>

static Preferences prefsWifi;

String contrasenaGuardada(const String& ssid) {
    prefsWifi.begin("wifi", true);
    String ssidGuardado = prefsWifi.getString("ssid", "");
    String password = (ssidGuardado == ssid) ? prefsWifi.getString("pass", "") : "";
    prefsWifi.end();
    return password;
}

void guardarCredenciales(const String& ssid, const String& password) {
    prefsWifi.begin("wifi", false);
    prefsWifi.putString("ssid", ssid);
    prefsWifi.putString("pass", password);
    prefsWifi.end();
}
