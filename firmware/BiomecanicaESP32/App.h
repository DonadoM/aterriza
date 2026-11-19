#pragma once
#include "Snapshot.h"
// Punto de entrada del firmware: llamados desde setup() y loop().
void appSetup();
void appLoop();

// Copia coherente de la última muestra y del estado de las IMU (para el nodo web).
// Segura de llamar desde cualquier tarea. Devuelve false si aún no hay muestras.
bool appSnapshot(Snapshot& out);
