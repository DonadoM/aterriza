// Nodo web del dispositivo (HTTP :80), todo con usuario y contraseña (Basic):
//   GET  /            panel: 3 indicadores de IMU + indicador de WiFi
//   GET  /api/status  las mismas lecturas en JSON
//   GET/POST /setup   usuario y contraseña de acceso y, opcional, red WiFi
//
// Primer uso: mientras no exista usuario y contraseña, /setup está abierto (conviene
// hacerlo conectado al punto de acceso WPA2 del dispositivo). Después exige credenciales.
// Tras 5 intentos fallidos seguidos bloquea los accesos 30 s. Las contraseñas no se
// guardan ni se imprimen: la NVS solo guarda un hash con sal.
// Limitación: HTTP sin cifrar (HTTPS no es práctico en el ESP32 para este uso); usa
// el punto de acceso propio o una red de confianza.
#pragma once

namespace webnode {

void begin();    // registra rutas y arranca el servidor (idempotente)
void handle();   // llamar desde loop()

}  // namespace webnode
