#include "WifiLink.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#if CFG_WIFI_ENABLED

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <lwip/sockets.h>

#include "NetConfig.h"
#include "WebNode.h"

namespace wifilink {

static WiFiServer server(WIFI_TCP_PORT);
static WiFiClient client;
static SemaphoreHandle_t mtx = nullptr;   // protege `client` (lo usan dos tareas)
static LogFn logFn = nullptr;
static uint32_t startMs = 0, lastRetryMs = 0;
static bool apMode = false;
static bool serverUp = false;
static bool staConn = false;
static uint32_t drops = 0, accepted = 0;

static void log(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
static void log(const char* fmt, ...) {
  if (!logFn) return;
  char buf[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  logFn(buf);
}

static bool staConfigured() { return netconfig::wifiSsid()[0] != '\0'; }

// Servicios de red (TCP y web): se levantan al tener cualquier interfaz activa.
static void startServices() {
  if (serverUp) return;
  server.begin();
  server.setNoDelay(true);
  serverUp = true;
  if (MDNS.begin(WIFI_HOSTNAME)) {
    MDNS.addService("aterriza", "tcp", WIFI_TCP_PORT);
    MDNS.addService("http", "tcp", WEB_PORT);
  }
  webnode::begin();
}

// Punto de acceso propio. Si hay una red configurada se mantiene también el intento
// de unirse a ella (AP+STA): al volver la red, el dispositivo se une solo.
static void startAp() {
  if (apMode) return;
  apMode = true;
  const bool both = staConfigured();
  WiFi.mode(both ? WIFI_AP_STA : WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
  if (both) WiFi.begin(netconfig::wifiSsid(), netconfig::wifiPass());
  startServices();
  log("#OK WiFi punto de acceso '%s' (clave en config.h) | panel http://%s/ | flujo %s:%u", WIFI_AP_SSID,
      WiFi.softAPIP().toString().c_str(), WiFi.softAPIP().toString().c_str(), (unsigned)WIFI_TCP_PORT);
}

void begin(LogFn fn) {
  logFn = fn;
  mtx = xSemaphoreCreateMutex();
  startMs = lastRetryMs = millis();
  WiFi.persistent(false);
  WiFi.setHostname(WIFI_HOSTNAME);
  WiFi.setAutoReconnect(true);
  if (!staConfigured()) {
    log("#INFO WiFi: sin red configurada (abre /setup en el punto de acceso), se crea un punto de acceso");
    startAp();
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);   // sin ahorro de energía: baja la latencia y evita pérdidas
  WiFi.begin(netconfig::wifiSsid(), netconfig::wifiPass());
  log("#INFO WiFi: conectando a la red '%s'...", netconfig::wifiSsid());
}

void poll() {
  if (!mtx) return;

  // --- unión a la red configurada: avisos, reintentos y respaldo a punto de acceso
  if (staConfigured()) {
    const bool now = WiFi.status() == WL_CONNECTED;
    if (now != staConn) {
      staConn = now;
      if (now) {
        startServices();
        log("#OK WiFi conectado a '%s' | %s | panel http://%s/ | flujo :%u (tambien %s.local)", netconfig::wifiSsid(),
            WiFi.localIP().toString().c_str(), WiFi.localIP().toString().c_str(), (unsigned)WIFI_TCP_PORT, WIFI_HOSTNAME);
      } else {
        log("#WARN WiFi: conexion perdida con '%s', reintentando", netconfig::wifiSsid());
        lastRetryMs = millis();
      }
    }
    if (!now) {
      if (!apMode && millis() - startMs > WIFI_STA_TIMEOUT_MS) {
        log("#WARN WiFi: no se pudo conectar a '%s', se crea un punto de acceso", netconfig::wifiSsid());
        startAp();
      }
      if (millis() - lastRetryMs > WIFI_RETRY_MS) {
        lastRetryMs = millis();
        WiFi.reconnect();
      }
    }
  }

  if (!serverUp) return;
  WiFiClient incoming = server.accept();
  if (incoming) {
    incoming.setNoDelay(true);
    if (xSemaphoreTake(mtx, pdMS_TO_TICKS(50)) == pdTRUE) {
      if (client) client.stop();   // el cliente más reciente reemplaza al anterior
      client = incoming;
      accepted++;
      xSemaphoreGive(mtx);
      log("#OK WiFi cliente conectado desde %s", incoming.remoteIP().toString().c_str());
    }
  }
}

void status(NetStatus& s) {
  memset(&s, 0, sizeof(s));
  s.staConfigured = staConfigured();
  s.wifiConnected = staConn;
  s.apActive = apMode;
  s.tcpClient = clientConnected();
  s.authConfigured = netconfig::hasWebCreds();
  s.uptimeS = millis() / 1000;
  strlcpy(s.ssid, netconfig::wifiSsid(), sizeof(s.ssid));
  if (staConn) {
    strlcpy(s.ip, WiFi.localIP().toString().c_str(), sizeof(s.ip));
    s.rssi = WiFi.RSSI();
  }
  if (apMode) {
    strlcpy(s.apSsid, WIFI_AP_SSID, sizeof(s.apSsid));
    strlcpy(s.apIp, WiFi.softAPIP().toString().c_str(), sizeof(s.apIp));
  }
}

bool clientConnected() {
  if (!mtx) return false;
  bool c = false;
  if (xSemaphoreTake(mtx, 0) == pdTRUE) {
    c = client && client.connected();
    xSemaphoreGive(mtx);
  }
  return c;
}

void write(const uint8_t* data, size_t n) {
  if (!mtx || !serverUp) return;
  if (xSemaphoreTake(mtx, 0) != pdTRUE) { drops++; return; }
  if (client && client.connected()) {
    // Envío no bloqueante: si el buffer de la conexión está lleno (cliente lento) el dato
    // se descarta. (availableForWrite() de WiFiClient devuelve siempre 0 en este core, por
    // eso no sirve para decidir.)
    const int fd = client.fd();
    if (fd < 0 || ::send(fd, data, n, MSG_DONTWAIT) != (ssize_t)n) drops++;
  }
  xSemaphoreGive(mtx);
}

int read() {
  if (!mtx || !serverUp) return -1;
  int c = -1;
  if (xSemaphoreTake(mtx, 0) == pdTRUE) {
    if (client && client.connected() && client.available()) c = client.read();
    xSemaphoreGive(mtx);
  }
  return c;
}

void describe(char* out, size_t size) {
  NetStatus s;
  status(s);
  const char* estado = s.wifiConnected ? "conectado" : s.apActive ? "punto de acceso" : s.staConfigured ? "conectando" : "sin red";
  int n = snprintf(out, size, "WiFi: %s", estado);
  if (s.wifiConnected) n += snprintf(out + n, size - n, " '%s' %s RSSI %d dBm", s.ssid, s.ip, s.rssi);
  if (s.apActive && (size_t)n < size) n += snprintf(out + n, size - n, " | AP '%s' %s", s.apSsid, s.apIp);
  if ((size_t)n < size)
    snprintf(out + n, size - n, " | flujo :%u cliente:%s aceptados:%lu descartes:%lu | acceso web:%s", (unsigned)WIFI_TCP_PORT,
             s.tcpClient ? "SI" : "NO", (unsigned long)accepted, (unsigned long)drops, s.authConfigured ? "configurado" : "SIN CONFIGURAR (/setup)");
}

}  // namespace wifilink

#else  // WiFi desactivado en la compilación (p. ej. Wokwi): el módulo no hace nada

namespace wifilink {
void begin(LogFn) {}
void poll() {}
void status(NetStatus& s) { memset(&s, 0, sizeof(s)); }
bool clientConnected() { return false; }
void write(const uint8_t*, size_t) {}
int read() { return -1; }
void describe(char* out, size_t size) { snprintf(out, size, "WiFi: desactivado en esta compilacion"); }
}  // namespace wifilink

#endif
