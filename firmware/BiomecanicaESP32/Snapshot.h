// Tipos de solo datos que comparten el muestreo, el WiFi y el nodo web.
// No dependen de Arduino: se compilan también en el PC (pruebas del JSON).
#pragma once
#include <stdint.h>
#include "config.h"

// Última muestra de una IMU. Unidades: aceleración en g, giro en °/s,
// campo magnético en µT, inclinación en grados (relativa a la tara).
struct ImuSnapshot {
  bool present;        // detectada en su bus I2C
  bool readError;      // la última lectura falló
  char model[12];      // "MPU6050", "GY-85", ...
  char mag[12];        // "QMC5883P", "HMC5883L", ... o "ninguno"
  uint8_t addr;        // dirección I2C del acelerómetro/giroscopio
  uint32_t i2cErrors;  // errores acumulados desde el arranque
  float acc[3];        // g
  float gyr[3];        // °/s
  float magf[3];       // µT
  float angle;         // ° (plano sagital, relativo a la tara)
};

struct Snapshot {
  bool valid;          // hay al menos una muestra
  uint32_t seq;
  uint32_t uptimeS;
  uint32_t overruns;
  float rateHz;        // frecuencia media de muestreo
  bool tared;
  uint16_t flags;
  float knee, trunk;   // ° (relativos a la tara)
  uint16_t fsr[NUM_FSR];  // cuentas del ADC (0-4095)
  ImuSnapshot imu[NUM_IMU];
};

// Estado de conectividad.
struct NetStatus {
  bool staConfigured;  // hay una red configurada
  bool wifiConnected;  // unido a esa red
  bool apActive;       // punto de acceso propio encendido
  bool tcpClient;      // hay un cliente en el flujo TCP
  bool authConfigured; // ya hay usuario y contraseña web
  int rssi;            // dBm (solo si wifiConnected)
  uint32_t uptimeS;
  char ssid[33];       // red a la que se une (o intenta unirse)
  char ip[16];         // IP en esa red
  char apSsid[33];
  char apIp[16];
};
