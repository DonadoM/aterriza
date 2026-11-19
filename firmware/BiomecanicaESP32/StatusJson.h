// Estado del dispositivo en JSON (lo sirve GET /api/status). C++ puro.
#pragma once
#include <stddef.h>
#include "Snapshot.h"

// Escribe el JSON en `out` (siempre terminado en '\0'). Devuelve su longitud,
// o 0 si no cupo en `cap` bytes. Los valores no medibles salen como null.
size_t buildStatusJson(const Snapshot& s, const NetStatus& n, char* out, size_t cap);
