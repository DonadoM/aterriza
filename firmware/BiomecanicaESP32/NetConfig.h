// Configuración de red y credenciales de acceso, guardadas en NVS (namespace propio,
// separado del de calibración: borrar una nunca toca la otra).
//
// Dos juegos de credenciales distintos:
//   - WiFi: red a la que se une el dispositivo (SSID y clave de ESE router).
//   - Acceso web: usuario y contraseña para consultar el dispositivo. Se guarda solo
//     un hash con sal (no la contraseña) y nunca se imprime en el puerto serie.
#pragma once

namespace netconfig {

void begin();   // carga de NVS; si está vacío, usa los valores iniciales de wifi_secrets.h

// --- WiFi
const char* wifiSsid();   // "" si no hay red configurada
const char* wifiPass();

// --- Acceso web
bool hasWebCreds();
bool verifyWeb(const char* user, const char* pass);

// Guardan y devuelven nullptr si salió bien, o el motivo del rechazo (texto para mostrar).
const char* setWeb(const char* user, const char* pass);
const char* setWifi(const char* ssid, const char* pass);   // ssid vacío = olvidar la red

void clear();   // olvida red y credenciales web (comando serie 'k')

}  // namespace netconfig
