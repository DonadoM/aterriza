// Enlace WiFi del firmware. Dos servicios sobre la misma conexión:
//   - TCP :3333  flujo CSV idéntico al del puerto serie + comandos de una letra
//                (lo usa la plataforma). Un solo cliente; el más reciente gana.
//   - HTTP :80   panel y API de lecturas con usuario y contraseña (WebNode).
// El USB sigue funcionando a la vez.
//
// Red: se une a la configurada (NetConfig). Si no hay red o no responde a tiempo,
// enciende además su propio punto de acceso y sigue intentando unirse (modo AP+STA);
// al volver la red se une sola.
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "Snapshot.h"

namespace wifilink {

// Función para publicar eventos como mensajes "#..." del firmware.
using LogFn = void (*)(const char* text);

void begin(LogFn log);   // arranca sin bloquear
void poll();             // llamar desde loop(): reconexión, respaldo AP, clientes nuevos
void status(NetStatus& out);   // consulta del estado de conectividad

bool clientConnected();

// Envío de mejor esfuerzo desde la tarea de transmisión: si el cliente va lento
// el dato se descarta (nunca se bloquea el muestreo) y se cuenta.
void write(const uint8_t* data, size_t n);

// Siguiente byte de comando recibido por TCP, o -1 si no hay.
int read();

// Resumen de una línea para el comando 'i'.
void describe(char* out, size_t size);

}  // namespace wifilink
