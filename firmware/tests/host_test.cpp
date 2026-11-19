// Pruebas en el PC del código del firmware que no depende de Arduino:
// SHA-256, autenticación Basic y JSON de estado.
// Compilar: c++ -std=gnu++17 -I BiomecanicaESP32 tests/host_test.cpp BiomecanicaESP32/StatusJson.cpp -o host_test   (en Windows: host_test.exe)
// Las líneas "JSON <caso> {...}" las valida plataforma/backend/tests/test_firmware_web.py.
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "AuthUtil.h"
#include "Sha256.h"
#include "Snapshot.h"
#include "StatusJson.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

static void hex(const uint8_t* h, char* out) { for (int i = 0; i < 32; i++) sprintf(out + 2 * i, "%02x", h[i]); }

static void testSha() {
  uint8_t h[32]; char s[65];
  sha256::hash((const uint8_t*)"", 0, h); hex(h, s);
  CHECK(!strcmp(s, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
  sha256::hash((const uint8_t*)"abc", 3, h); hex(h, s);
  CHECK(!strcmp(s, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
  const char* m = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";  // cruza el límite de bloque
  sha256::hash((const uint8_t*)m, strlen(m), h); hex(h, s);
  CHECK(!strcmp(s, "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"));
}

static void testAuth() {
  char u[40], p[72];
  CHECK(authutil::parseBasic("Basic dXN1YXJpbzpjbGF2ZTEyMzQ=", u, sizeof(u), p, sizeof(p)));
  CHECK(!strcmp(u, "usuario") && !strcmp(p, "clave1234"));
  CHECK(authutil::parseBasic("basic dXN1YXJpbzpjbGF2ZTEyMzQ=", u, sizeof(u), p, sizeof(p)));      // esquema sin distinguir mayúsculas
  CHECK(authutil::parseBasic("Basic dTpwOndpdGg6Y29sb24=", u, sizeof(u), p, sizeof(p)));           // "u:p:with:colon"
  CHECK(!strcmp(u, "u") && !strcmp(p, "p:with:colon"));
  CHECK(!authutil::parseBasic("Bearer abc", u, sizeof(u), p, sizeof(p)));
  CHECK(!authutil::parseBasic("Basic !!!", u, sizeof(u), p, sizeof(p)));
  CHECK(!authutil::parseBasic("Basic c2luZG9zcHVudG9z", u, sizeof(u), p, sizeof(p)));              // sin ':'
  CHECK(!authutil::parseBasic(nullptr, u, sizeof(u), p, sizeof(p)));
  CHECK(!authutil::parseBasic("Basic dXN1YXJpbzpjbGF2ZTEyMzQ=", u, 4, p, sizeof(p)));              // usuario no cabe
  CHECK(authutil::constantTimeStrEq("abc", "abc"));
  CHECK(!authutil::constantTimeStrEq("abc", "abd"));
  CHECK(!authutil::constantTimeStrEq("abc", "abcd"));
  CHECK(!authutil::constantTimeStrEq("", "a"));
}

static void emit(const char* name, const Snapshot& s, const NetStatus& n) {
  static char out[4096];
  const size_t len = buildStatusJson(s, n, out, sizeof(out));
  CHECK(len > 0 && len == strlen(out));
  printf("JSON %s %s\n", name, out);
}

int main() {
  testSha();
  testAuth();

  // 1) Placa real: muslo MPU6050+QMC5883P, pierna GY-85+HMC5883L, lumbar ausente, conectada a WiFi.
  Snapshot s; memset(&s, 0, sizeof(s));
  NetStatus n; memset(&n, 0, sizeof(n));
  s.valid = true; s.rateHz = 99.8f; s.tared = true; s.uptimeS = 125; s.knee = 12.5f; s.trunk = 0;
  s.imu[0] = {true, false, "MPU6050", "QMC5883P", 0x68, 0, {0.01f, -0.02f, 0.99f}, {0.1f, 0.2f, -0.3f}, {20.5f, -5.1f, 40.0f}, 3.2f};
  s.imu[1] = {true, false, "GY-85", "HMC5883L", 0x53, 2, {0.0f, 0.0f, 1.0f}, {0, 0, 0}, {1, 2, 3}, -1.0f};
  n.staConfigured = n.wifiConnected = true; n.authConfigured = true; n.rssi = -61; n.uptimeS = 125;
  strcpy(n.ssid, "Red Casa"); strcpy(n.ip, "192.168.1.50");
  emit("conectado", s, n);

  // 2) Valores no medibles (NaN/Inf) y SSID con comillas y barra invertida: el JSON debe seguir siendo válido.
  Snapshot t = s;
  t.imu[0].acc[0] = NAN; t.imu[0].gyr[1] = INFINITY; t.imu[0].angle = NAN;
  NetStatus m; memset(&m, 0, sizeof(m));
  m.apActive = true; m.staConfigured = true;
  strcpy(m.ssid, "Mi \"red\" \\ casa"); strcpy(m.apSsid, "Aterriza-ESP32"); strcpy(m.apIp, "192.168.4.1");
  emit("ap_nan_comillas", t, m);

  // 3) Sin red configurada ni credenciales, sin muestras todavía.
  Snapshot z; memset(&z, 0, sizeof(z));
  NetStatus e; memset(&e, 0, sizeof(e));
  emit("vacio", z, e);

  // Truncamiento: con un búfer pequeño devuelve 0 y no escribe fuera.
  char small[40]; memset(small, 'X', sizeof(small));
  CHECK(buildStatusJson(s, n, small, 32) == 0);
  CHECK(small[39] == 'X');

  printf(failures ? "FALLARON %d\n" : "OK\n", failures);
  return failures ? 1 : 0;
}
