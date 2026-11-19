#include "NetConfig.h"

#include <Arduino.h>
#include <Preferences.h>
#include <esp_system.h>

#include "AuthUtil.h"
#include "Sha256.h"
#include "config.h"

namespace netconfig {

static constexpr const char* NS = "aterriza_net";
static constexpr int HASH_ROUNDS = 1000;   // endurece el hash frente a un volcado de la NVS

static char ssid_[33] = "";
static char wpass_[64] = "";
static char user_[33] = "";
static uint8_t salt_[16] = {0};
static uint8_t hash_[32] = {0};
static bool hasWeb_ = false;

// hash = SHA-256 iterado sobre (sal || contraseña)
static void derive(const uint8_t* salt, const char* pass, uint8_t out[32]) {
  uint8_t buf[16 + 32 + 64];
  const size_t pl = strnlen(pass, 64);
  memcpy(buf, salt, 16);
  memcpy(buf + 16 + 32, pass, pl);
  uint8_t h[32] = {0};
  for (int i = 0; i < HASH_ROUNDS; i++) {
    memcpy(buf + 16, h, 32);
    sha256::hash(buf, 16 + 32 + pl, h);
  }
  memcpy(out, h, 32);
  memset(buf, 0, sizeof(buf));
}

static bool validName(const char* s, size_t minLen, size_t maxLen) {
  const size_t n = strlen(s);
  if (n < minLen || n > maxLen) return false;
  for (size_t i = 0; i < n; i++) {
    const char c = s[i];
    const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
    if (!ok) return false;
  }
  return true;
}

static void saveWeb(Preferences& p) {
  p.putString("wuser", user_);
  p.putBytes("wsalt", salt_, sizeof(salt_));
  p.putBytes("whash", hash_, sizeof(hash_));
}

const char* setWeb(const char* user, const char* pass) {
  if (!validName(user, 3, 32)) return "El usuario debe tener de 3 a 32 caracteres: letras, números, punto, guion o guion bajo.";
  const size_t pl = strlen(pass);
  if (pl < WEB_MIN_PASS_LEN || pl > 64) return "La contraseña debe tener de 8 a 64 caracteres.";
  strlcpy(user_, user, sizeof(user_));
  for (size_t i = 0; i < sizeof(salt_); i += 4) {
    const uint32_t r = esp_random();
    memcpy(salt_ + i, &r, 4);
  }
  derive(salt_, pass, hash_);
  hasWeb_ = true;
  Preferences p;
  if (!p.begin(NS, false)) return "No se pudo guardar en la memoria del dispositivo.";
  saveWeb(p);
  p.end();
  return nullptr;
}

const char* setWifi(const char* ssid, const char* pass) {
  const size_t sl = strlen(ssid), pl = strlen(pass);
  if (sl > 32) return "El nombre de la red WiFi admite hasta 32 caracteres.";
  if (sl > 0 && pl != 0 && (pl < 8 || pl > 63)) return "La clave del WiFi debe tener de 8 a 63 caracteres (o estar vacía si la red es abierta).";
  strlcpy(ssid_, ssid, sizeof(ssid_));
  strlcpy(wpass_, pass, sizeof(wpass_));
  Preferences p;
  if (!p.begin(NS, false)) return "No se pudo guardar en la memoria del dispositivo.";
  p.putString("ssid", ssid_);
  p.putString("wpass", wpass_);
  p.end();
  return nullptr;
}

void begin() {
  Preferences p;
  if (p.begin(NS, false)) {
    p.getString("ssid", ssid_, sizeof(ssid_));
    p.getString("wpass", wpass_, sizeof(wpass_));
    p.getString("wuser", user_, sizeof(user_));
    hasWeb_ = p.getBytes("wsalt", salt_, sizeof(salt_)) == sizeof(salt_) &&
              p.getBytes("whash", hash_, sizeof(hash_)) == sizeof(hash_) && user_[0] != '\0';
    p.end();
  }
  // Valores iniciales de wifi_secrets.h (archivo local fuera del repositorio): solo si la NVS está vacía.
  if (ssid_[0] == '\0' && WIFI_STA_SSID[0] != '\0') setWifi(WIFI_STA_SSID, WIFI_STA_PASS);
  if (!hasWeb_ && WEB_USER[0] != '\0' && WEB_PASS[0] != '\0') setWeb(WEB_USER, WEB_PASS);
}

const char* wifiSsid() { return ssid_; }
const char* wifiPass() { return wpass_; }
bool hasWebCreds() { return hasWeb_; }

bool verifyWeb(const char* user, const char* pass) {
  if (!hasWeb_) return false;
  uint8_t h[32];
  derive(salt_, pass, h);
  // Se evalúan las dos comparaciones siempre (sin salir temprano).
  const bool okUser = authutil::constantTimeStrEq(user, user_);
  const bool okPass = authutil::constantTimeEq(h, hash_, sizeof(h));
  memset(h, 0, sizeof(h));
  return okUser & okPass;
}

void clear() {
  Preferences p;
  if (p.begin(NS, false)) {
    p.clear();
    p.end();
  }
  ssid_[0] = wpass_[0] = user_[0] = '\0';
  memset(salt_, 0, sizeof(salt_));
  memset(hash_, 0, sizeof(hash_));
  hasWeb_ = false;
}

}  // namespace netconfig
