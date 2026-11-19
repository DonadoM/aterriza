#include "StatusJson.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace {

struct Out {
  char* buf;
  size_t cap;
  size_t len;
  bool full;
  void put(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
};

void Out::put(const char* fmt, ...) {
  if (full) return;
  va_list a;
  va_start(a, fmt);
  const int n = vsnprintf(buf + len, cap - len, fmt, a);
  va_end(a);
  if (n < 0 || (size_t)n >= cap - len) {
    full = true;
    return;
  }
  len += (size_t)n;
}

void str(Out& o, const char* s) {
  o.put("\"");
  for (; *s; ++s) {
    const unsigned char c = (unsigned char)*s;
    if (c == '"' || c == '\\') o.put("\\%c", c);
    else if (c < 0x20) o.put("\\u%04x", c);
    else o.put("%c", c);
  }
  o.put("\"");
}

void num(Out& o, float v, int decimals) {
  if (isfinite(v)) o.put("%.*f", decimals, (double)v);
  else o.put("null");
}

// {"x":..,"y":..,"z":..,"mag":..}: nulos si la IMU no está disponible.
void vec(Out& o, const float v[3], bool valid, int d) {
  if (!valid) {
    o.put("null");
    return;
  }
  o.put("{\"x\":");
  num(o, v[0], d);
  o.put(",\"y\":");
  num(o, v[1], d);
  o.put(",\"z\":");
  num(o, v[2], d);
  o.put(",\"mag\":");
  num(o, sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]), d);
  o.put("}");
}

const char* const NOMBRES[NUM_IMU] = {"Muslo", "Pierna", "Lumbar"};

}  // namespace

size_t buildStatusJson(const Snapshot& s, const NetStatus& n, char* out, size_t cap) {
  if (!out || cap < 2) return 0;
  Out o{out, cap, 0, false};
  out[0] = '\0';

  const char* wifi = n.wifiConnected ? "conectado" : n.apActive ? "punto_acceso" : n.staConfigured ? "conectando" : "sin_red";

  o.put("{\"fw\":");
  str(o, FW_VERSION);
  o.put(",\"uptime_s\":%lu,\"muestreo\":{\"hz\":", (unsigned long)s.uptimeS);
  num(o, s.rateHz, 1);
  o.put(",\"overruns\":%lu,\"tara\":%s}", (unsigned long)s.overruns, s.tared ? "true" : "false");

  o.put(",\"wifi\":{\"estado\":");
  str(o, wifi);
  o.put(",\"ssid\":");
  str(o, n.ssid);
  o.put(",\"ip\":");
  str(o, n.ip);
  o.put(",\"rssi_dbm\":");
  if (n.wifiConnected) o.put("%d", n.rssi);
  else o.put("null");
  o.put(",\"ap_activo\":%s,\"ap_ssid\":", n.apActive ? "true" : "false");
  str(o, n.apSsid);
  o.put(",\"ap_ip\":");
  str(o, n.apIp);
  o.put(",\"cliente_tcp\":%s,\"acceso_configurado\":%s}", n.tcpClient ? "true" : "false",
        n.authConfigured ? "true" : "false");

  o.put(",\"imu\":[");
  for (uint8_t i = 0; i < NUM_IMU; i++) {
    const ImuSnapshot& m = s.imu[i];
    const bool live = s.valid && m.present;
    const char* estado = !m.present ? "no_conectada" : m.readError ? "sin_respuesta" : "ok";
    if (i) o.put(",");
    o.put("{\"id\":");
    str(o, IMU_NAMES[i]);
    o.put(",\"nombre\":");
    str(o, NOMBRES[i]);
    o.put(",\"estado\":");
    str(o, estado);
    o.put(",\"modelo\":");
    if (m.present) str(o, m.model);
    else o.put("null");
    o.put(",\"magnetometro\":");
    if (m.present) str(o, m.mag);
    else o.put("null");
    o.put(",\"direccion_i2c\":");
    if (m.present) o.put("\"0x%02X\"", m.addr);
    else o.put("null");
    o.put(",\"errores_i2c\":%lu,\"acel_g\":", (unsigned long)m.i2cErrors);
    vec(o, m.acc, live, 3);
    o.put(",\"giro_dps\":");
    vec(o, m.gyr, live, 1);
    o.put(",\"mag_uT\":");
    vec(o, m.magf, live && strcmp(m.mag, "ninguno") != 0, 1);
    o.put(",\"inclinacion_deg\":");
    if (live) num(o, m.angle, 1);
    else o.put("null");
    o.put("}");
  }
  o.put("],\"fsr_cuentas\":[%u,%u,%u,%u],\"rodilla_deg\":", s.fsr[0], s.fsr[1], s.fsr[2], s.fsr[3]);
  num(o, s.valid ? s.knee : NAN, 1);
  o.put(",\"tronco_deg\":");
  num(o, s.valid ? s.trunk : NAN, 1);
  o.put("}");

  if (o.full) {
    out[0] = '\0';
    return 0;
  }
  return o.len;
}
