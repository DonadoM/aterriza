#include "WebNode.h"

#include "config.h"

#if CFG_WIFI_ENABLED

#include <Arduino.h>
#include <WebServer.h>
#include <string.h>

#include "App.h"
#include "AuthUtil.h"
#include "Dashboard.h"
#include "NetConfig.h"
#include "Snapshot.h"
#include "StatusJson.h"
#include "WifiLink.h"

namespace webnode {

static WebServer server(WEB_PORT);
static bool started = false;
static uint8_t fails = 0;
static uint32_t lockUntilMs = 0;
static bool locked = false;
static uint32_t restartAtMs = 0;   // reinicio diferido tras cambiar la red

static void noStore() {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("X-Content-Type-Options", "nosniff");
  server.sendHeader("X-Frame-Options", "DENY");
}

static void sendText(int code, const char* type, const char* body) {
  noStore();
  server.send(code, type, body);
}

static void sendProgmem(const char* type, PGM_P body) {
  noStore();
  server.send_P(200, type, body);
}

// true si la petición trae credenciales válidas. Si no, ya respondió (401 o 429).
static bool authorized() {
  const uint32_t now = millis();
  if (locked) {
    if ((int32_t)(now - lockUntilMs) < 0) {
      char retry[8];
      snprintf(retry, sizeof(retry), "%lu", (unsigned long)((lockUntilMs - now) / 1000 + 1));
      server.sendHeader("Retry-After", retry);
      sendText(429, "text/plain; charset=utf-8", "Demasiados intentos fallidos. Espera unos segundos.");
      return false;
    }
    locked = false;
    fails = 0;
  }

  char user[40], pass[72];
  bool ok = false;
  if (server.hasHeader("Authorization") &&
      authutil::parseBasic(server.header("Authorization").c_str(), user, sizeof(user), pass, sizeof(pass))) {
    ok = netconfig::verifyWeb(user, pass);
  }
  memset(pass, 0, sizeof(pass));
  if (ok) {
    fails = 0;
    return true;
  }
  // Una petición sin encabezado es la primera visita del navegador: no cuenta como fallo.
  if (server.hasHeader("Authorization") && ++fails >= WEB_MAX_FAILS) {
    locked = true;
    lockUntilMs = now + WEB_LOCK_MS;
  }
  server.sendHeader("WWW-Authenticate", "Basic realm=\"Aterriza\", charset=\"UTF-8\"");
  sendText(401, "text/plain; charset=utf-8", "Se requiere usuario y contrasena.");
  return false;
}

// Mientras no existan credenciales, todo lleva a /setup (primer uso).
static bool needSetup() {
  if (netconfig::hasWebCreds()) return false;
  server.sendHeader("Location", "/setup");
  sendText(302, "text/plain; charset=utf-8", "Configura el acceso en /setup");
  return true;
}

static void handleRoot() {
  if (needSetup() || !authorized()) return;
  sendProgmem("text/html; charset=utf-8", DASHBOARD_HTML);
}

static void handleStyle() {
  // Solo estilos, sin datos del dispositivo: se sirve sin autenticación para que la
  // página de acceso y la de configuración se vean bien.
  noStore();
  server.send_P(200, "text/css; charset=utf-8", STYLE_HTML);
}

static void handleStatus() {
  if (needSetup() || !authorized()) return;
  static char json[2048];
  Snapshot snap;
  NetStatus net;
  appSnapshot(snap);
  wifilink::status(net);
  const size_t n = buildStatusJson(snap, net, json, sizeof(json));
  if (n == 0) {
    sendText(500, "application/json", "{\"error\":\"estado demasiado grande\"}");
    return;
  }
  sendText(200, "application/json; charset=utf-8", json);
}

static void sendSetupResult(int code, const char* title, const char* msg, bool again) {
  // Los textos que se insertan son fijos o provienen de netconfig (sin entrada del usuario).
  static char page[900];
  snprintf(page, sizeof(page),
           "<!doctype html><html lang=\"es\"><head><meta charset=\"utf-8\">"
           "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
           "<title>Aterriza</title><link rel=\"stylesheet\" href=\"/style.css\"></head>"
           "<body><div class=\"wrap\"><h1>%s</h1><p>%s</p><p><a href=\"%s\">%s</a></p></div></body></html>",
           title, msg, again ? "/setup" : "/", again ? "Volver al formulario" : "Ir al panel");
  sendText(code, "text/html; charset=utf-8", page);
}

static void handleSetupGet() {
  // Con credenciales ya configuradas se exige autenticación; sin ellas, es el primer uso.
  if (netconfig::hasWebCreds() && !authorized()) return;
  sendProgmem("text/html; charset=utf-8", SETUP_HTML);
}

static void handleSetupPost() {
  const bool first = !netconfig::hasWebCreds();
  if (!first && !authorized()) return;

  const String user = server.arg("user");
  const String pass = server.arg("pass");
  const String ssid = server.arg("ssid");
  const String wpass = server.arg("wpass");

  const char* err = netconfig::setWeb(user.c_str(), pass.c_str());
  if (err) {
    sendSetupResult(400, "No se guardó", err, true);
    return;
  }
  bool wifiChanged = false;
  if (ssid.length() > 0) {
    err = netconfig::setWifi(ssid.c_str(), wpass.c_str());
    if (err) {
      sendSetupResult(400, "Acceso guardado, red no", err, true);
      return;
    }
    wifiChanged = true;
  }
  fails = 0;
  locked = false;
  if (wifiChanged) {
    restartAtMs = millis() + 1500;
    sendSetupResult(200, "Guardado",
                    "El dispositivo se reinicia para unirse a la red. Vuelve a abrir el panel con la nueva dirección.", false);
  } else {
    sendSetupResult(200, "Guardado", "Acceso configurado.", false);
  }
}

static void handleNotFound() {
  sendText(404, "text/plain; charset=utf-8", "No encontrado");
}

void begin() {
  if (started) return;
  started = true;
  static const char* keys[] = {"Authorization"};
  server.collectHeaders(keys, 1);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/style.css", HTTP_GET, handleStyle);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/setup", HTTP_GET, handleSetupGet);
  server.on("/setup", HTTP_POST, handleSetupPost);
  server.onNotFound(handleNotFound);
  server.begin();
}

void handle() {
  if (!started) return;
  server.handleClient();
  if (restartAtMs && (int32_t)(millis() - restartAtMs) >= 0) ESP.restart();
}

}  // namespace webnode

#else  // WiFi desactivado: sin nodo web

namespace webnode {
void begin() {}
void handle() {}
}  // namespace webnode

#endif
