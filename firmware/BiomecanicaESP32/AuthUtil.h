// Utilidades de autenticación HTTP Basic. C++ puro (se prueba en el PC).
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace authutil {

inline int b64val(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+') return 62;
  if (c == '/') return 63;
  return -1;
}

// Decodifica base64 (con o sin relleno '='). Devuelve la longitud, o -1 si es inválido o no cabe.
inline int b64decode(const char* in, uint8_t* out, size_t cap) {
  size_t n = 0;
  uint32_t acc = 0;
  int bits = 0;
  for (; *in && *in != '='; ++in) {
    const int v = b64val(*in);
    if (v < 0) return -1;
    acc = (acc << 6) | (uint32_t)v;
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      if (n >= cap) return -1;
      out[n++] = (uint8_t)((acc >> bits) & 0xFF);
    }
  }
  return (int)n;
}

// Compara el esquema sin distinguir mayúsculas. Propio, porque strncasecmp es POSIX y no
// existe en todos los compiladores (p. ej. MSVC).
inline bool startsWithBasic(const char* s) {
  static const char kBasic[] = "basic";
  for (int i = 0; i < 5; i++) {
    const char c = s[i];
    if (c == 0) return false;
    if ((c >= 'A' && c <= 'Z' ? c + 32 : c) != kBasic[i]) return false;
  }
  return true;
}

// "Basic dXN1YXJpbzpjbGF2ZQ==" → user, pass. false si el encabezado no es válido
// o si algún campo no cabe (los buffers incluyen el '\0').
inline bool parseBasic(const char* header, char* user, size_t userCap, char* pass, size_t passCap) {
  if (!header || !startsWithBasic(header) || (header[5] != ' ' && header[5] != '\t')) return false;
  const char* b = header + 5;
  while (*b == ' ' || *b == '\t') ++b;
  uint8_t raw[160];
  const int n = b64decode(b, raw, sizeof(raw) - 1);
  if (n < 1) return false;
  raw[n] = 0;
  const char* sep = (const char*)memchr(raw, ':', (size_t)n);
  if (!sep) return false;
  const size_t ul = (size_t)(sep - (const char*)raw);
  const size_t pl = (size_t)n - ul - 1;
  if (ul + 1 > userCap || pl + 1 > passCap) return false;
  memcpy(user, raw, ul);
  user[ul] = 0;
  memcpy(pass, sep + 1, pl);
  pass[pl] = 0;
  memset(raw, 0, sizeof(raw));
  return true;
}

// Compara en tiempo constante respecto al contenido (no sale al primer byte distinto).
inline bool constantTimeEq(const uint8_t* a, const uint8_t* b, size_t n) {
  uint8_t d = 0;
  for (size_t i = 0; i < n; i++) d |= (uint8_t)(a[i] ^ b[i]);
  return d == 0;
}

inline bool constantTimeStrEq(const char* a, const char* b) {
  const size_t la = strlen(a), lb = strlen(b);
  const size_t n = la > lb ? la : lb;
  uint8_t d = (uint8_t)(la != lb);
  for (size_t i = 0; i < n; i++) d |= (uint8_t)((i < la ? a[i] : 0) ^ (i < lb ? b[i] : 0));
  return d == 0;
}

}  // namespace authutil
