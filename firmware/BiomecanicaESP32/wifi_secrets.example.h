// Opcional. Copia este archivo como wifi_secrets.h (queda fuera del repositorio).
// Lo normal es NO usarlo: la red y el usuario/contraseña web se configuran desde
// http://192.168.4.1/setup (punto de acceso del dispositivo), sin recompilar, y se
// guardan en la NVS. Estos valores solo se usan si la NVS está vacía.
// Si la red no responde en WIFI_STA_TIMEOUT_MS, el ESP32 crea su propio punto de acceso.
#pragma once

// Red WiFi a la que se une el dispositivo (2,4 GHz).
// #define WIFI_STA_SSID "NombreDeLaRed"
// #define WIFI_STA_PASS "ClaveDeLaRed"

// Clave del punto de acceso propio (mínimo 8 caracteres). Cambia la pública por defecto.
// #define WIFI_AP_PASS_SET "pon-una-clave-propia"

// Acceso al panel web. DISTINTO de la clave del WiFi. No dejes valores reales en el repositorio.
// #define WEB_USER "usuario"
// #define WEB_PASS "contraseña-de-8-o-mas"
