# Guía de conexión del dispositivo (USB, WiFi y plataforma)

Paso a paso para conectar la placa por USB y por WiFi, cargar el firmware, hacer copias de
seguridad y usar la plataforma. Está escrita a partir de lo que se verificó con la placa real
(firmware 0.3.0). Al final se indica qué sigue sin probar.

## 0. Resumen

| Vía | Para qué | Cómo |
|---|---|---|
| USB | Cargar firmware, comandos, backup, diagnóstico | Cable de datos, puerto serie (ver tabla de la sección 1), 921600 baudios |
| WiFi (punto de acceso de la placa) | Panel web y flujo de datos sin cable | Red `Aterriza-ESP32`, panel `http://192.168.4.1/`, flujo TCP `192.168.4.1:3333` |
| Plataforma | Medir y analizar | Preparación → ESP32 por WiFi, host `192.168.4.1` |

La placa funciona con USB y WiFi a la vez.

## 1. Conexión por USB

1. Conecta la placa al computador con un cable USB **de datos** (los cables solo de carga no
   sirven).
2. Busca el puerto. El nombre cambia según el sistema (en los ejemplos de esta guía se escribe
   `PUERTO`; sustitúyelo por el tuyo):

   | Sistema | Cómo verlo | Ejemplo |
   |---|---|---|
   | macOS | `ls /dev/cu.*` | `/dev/cu.usbserial-0001` |
   | Linux | `ls /dev/ttyUSB* /dev/ttyACM*` | `/dev/ttyUSB0` |
   | Windows | Administrador de dispositivos → Puertos (COM y LPT), o `arduino-cli board list` | `COM3` |

   - **Linux:** si da "permission denied", añade tu usuario al grupo del puerto y vuelve a iniciar
     sesión: `sudo usermod -aG dialout $USER` (en algunas distribuciones el grupo es `uucp`).
   - **Windows:** si el puerto no aparece, instala el driver del conversor USB-serie de tu placa
     (CP210x de Silicon Labs o CH340/CH341 de WCH) y prueba otro cable de datos.
   - En esta placa, en Mac, el puerto es `/dev/cu.usbserial-0001`.
3. Abre el monitor serie a **921600 baudios** (por ejemplo con `arduino-cli monitor -p PUERTO -c baudrate=921600`).
   A 115200 se ve basura: es normal.
4. Comandos de una letra: `h` ayuda, `i` estado (IMU, WiFi, errores), `c` calibraciones,
   `s`/`x` iniciar/detener datos, `t` tara, `g` giroscopio, `a` acelerómetro, `m` magnetómetro,
   `r` borrar calibraciones de IMU, `k` olvidar red WiFi y usuario web.

Al abrir el puerto desde un programa, la placa puede **reiniciarse**. Al arrancar recalibra el
giroscopio (los 2 s de reposo) y guarda el resultado: déjala quieta en ese momento.

## 2. Copia de seguridad (hazla antes de cargar firmware)

Solo lee la placa; no la modifica. La herramienta `esptool` viene con el core de ESP32 instalado por Arduino (versión 5.3.1 en el
equipo donde se probó; la carpeta lleva el número de versión que tengas instalado):

| Sistema | Ruta de `esptool` |
|---|---|
| macOS | `~/Library/Arduino15/packages/esp32/tools/esptool_py/<versión>/esptool` |
| Linux | `~/.arduino15/packages/esp32/tools/esptool_py/<versión>/esptool` |
| Windows | `%LOCALAPPDATA%\Arduino15\packages\esp32\tools\esptool_py\<versión>\esptool.exe` |

macOS y Linux (bash):

```
ESPTOOL=~/Library/Arduino15/packages/esp32/tools/esptool_py/5.3.1/esptool   # en Linux: ~/.arduino15/...
mkdir -p ~/Backups/aterriza
$ESPTOOL --port PUERTO --baud 460800 read-flash 0 0x400000 ~/Backups/aterriza/backup_completo.bin
shasum -a 256 ~/Backups/aterriza/backup_completo.bin        # en Linux: sha256sum
```

Windows (PowerShell):

```
$esptool = "$env:LOCALAPPDATA\Arduino15\packages\esp32\tools\esptool_py\5.3.1\esptool.exe"
New-Item -ItemType Directory -Force "$HOME\Backups\aterriza" | Out-Null
& $esptool --port COM3 --baud 460800 read-flash 0 0x400000 "$HOME\Backups\aterriza\backup_completo.bin"
Get-FileHash "$HOME\Backups\aterriza\backup_completo.bin" -Algorithm SHA256
```

- La flash es de 4 MB (`0x400000`). Tarda unos 100 s.
- La copia incluye el firmware, la calibración (NVS) y la configuración de red.
- Los comandos de esta versión llevan **guiones** (`read-flash`, `write-flash`), no guion bajo.

**Restaurar** (deja la placa como estaba, calibración incluida). Mismo `esptool` y mismo `PUERTO`:

```
$ESPTOOL --port PUERTO --baud 460800 write-flash 0 ~/Backups/aterriza/backup_completo.bin
```
(en Windows: `& $esptool --port COM3 --baud 460800 write-flash 0 "$HOME\Backups\aterriza\backup_completo.bin"`)

Guarda además la salida del comando `c` por serie: es la calibración en texto legible.

## 3. Cargar el firmware

Desde la carpeta `firmware/` (igual en los tres sistemas; sustituye `PUERTO`):

```
arduino-cli compile --fqbn esp32:esp32:esp32 BiomecanicaESP32
arduino-cli upload -p PUERTO --fqbn esp32:esp32:esp32 BiomecanicaESP32
```

- Necesita el core `esp32:esp32` (probado con 4.0.0). No requiere librerías externas.
- `upload` **no compila**: sube lo último que haya en la caché. Si antes compilaste con otras
  opciones (por ejemplo `-DCFG_WIFI_ENABLED=0` para la simulación), subirías esa versión. Compila
  siempre justo antes de subir, y comprueba que el comando `i` muestre `WiFi: punto de acceso`
  y no `WiFi: desactivado en esta compilacion`.
- Subir la aplicación no borra la calibración ni la configuración de red (viven en otra
  partición).

## 4. Acceso por WiFi a la placa

La placa crea su propia red cuando no tiene otra configurada.

1. Une el computador a la red **`Aterriza-ESP32`**. La clave por defecto es pública (está en
   `config.h`); cámbiala en `wifi_secrets.h` si el equipo sale del laboratorio.
2. Comprueba que tu IP empiece por `192.168.4.`:
   macOS: Ajustes → WiFi → Detalles · Linux: `ip -4 addr` · Windows: `ipconfig`.
3. Abre `http://192.168.4.1/` escribiendo `http://` completo (algunos navegadores lo cambian a
   `https://`, que no existe aquí).
4. La primera vez te lleva a `/setup`: define **usuario y contraseña de acceso** (mínimo 8
   caracteres, distintos de los del WiFi). La red WiFi del laboratorio es opcional (solo 2,4 GHz).
5. A partir de ahí el panel y `/api/status` piden usuario y contraseña. Tras 5 intentos fallidos
   se bloquea 30 s.
6. Si olvidas la contraseña: comando `k` por USB (olvida red y credenciales web; no toca la
   calibración).

**Importante:** mientras estés en `Aterriza-ESP32` no hay internet. Si necesitas internet a la vez,
comparte la conexión del móvil **por cable USB** (no por WiFi, que ocuparía la misma antena) o
trabaja por turnos. En Windows y Linux, si tienes puerto de red, también sirve conectar el cable
Ethernet para internet y dejar el WiFi para la placa. El nombre `aterriza.local` depende de mDNS
(en Windows suele requerir Bonjour): por eso la guía usa siempre la IP `192.168.4.1`.

## 5. Usar la plataforma por WiFi

1. Arranca la plataforma. Funciona sin internet una vez instaladas las dependencias:
   - macOS / Linux: `cd plataforma && ./iniciar.sh` (o doble clic en `Abrir Aterriza.command` en Mac).
   - Windows: `cd plataforma; .\iniciar.ps1`, o doble clic en `Abrir Aterriza (Windows).cmd`.
   - La primera vez necesita internet para instalar `uv` y las dependencias de Python y Node.
2. Con la Mac conectada a `Aterriza-ESP32`, abre la plataforma → Preparación → ESP32 por WiFi.
3. Escribe el host **`192.168.4.1`** (no `aterriza.local`: el nombre `.local` no se resolvió en el
   punto de acceso) y pulsa Conectar.

## 6. Diagnóstico: qué se encontró y cómo se resolvió

| Síntoma | Causa | Solución |
|---|---|---|
| `nodename nor servname provided` | Host `aterriza.local` sin resolver | Usar `192.168.4.1` |
| `Network is unreachable` | La Mac no estaba en `Aterriza-ESP32` (su IP era la de otra red) | Unir la Mac a esa red y revisar que la IP sea `192.168.4.x` |
| Conecta pero no llegan datos | Un fallo de mi código: `availableForWrite()` de `WiFiClient` devuelve 0 en este core y se descartaban todos los datos | Corregido en `WifiLink.cpp` con envío no bloqueante |
| Tras subir el firmware el WiFi no está | Se subió una compilación en caché sin WiFi | Compilar justo antes de subir |

Prueba rápida de red, ya conectado a `Aterriza-ESP32`.

macOS y Linux (`nc` ya viene instalado; en algunas distribuciones se llama `netcat`):

```
nc -vz -w 3 192.168.4.1 80      # panel web
nc -vz -w 3 192.168.4.1 3333    # flujo de datos
nc -w 3 192.168.4.1 3333 | head -c 400    # deben salir líneas con números
```

Windows (PowerShell, no hay `nc` por defecto):

```
Test-NetConnection 192.168.4.1 -Port 80      # TcpTestSucceeded : True
Test-NetConnection 192.168.4.1 -Port 3333
```
Para ver las líneas de datos en Windows, usa la propia plataforma o el monitor serie por USB.

## 7. Cambios realizados en el proyecto

**Firmware** (`firmware/BiomecanicaESP32/`): `WifiLink.*` (conexión, reconexión, punto de acceso,
servidor TCP), `NetConfig.*` (red y credenciales en NVS, hash con sal), `WebNode.*` y `Dashboard.h`
(panel web autenticado), `StatusJson.*` y `Snapshot.h` (JSON de lecturas), `AuthUtil.h`, `Sha256.h`,
`App.*` (instantánea de lecturas y comando `k`), `config.h`, `wifi_secrets.example.h`. Detalle y
seguridad: `firmware/README.md`.

**Plataforma:** fuente "ESP32 por WiFi" (`laboratorio/core/link.py`, `plataforma/backend/app/bridge.py`
y `main.py`), pantalla de Preparación y barra superior del frontend.

**Pruebas:** `firmware/tests/host_test.cpp` y `plataforma/backend/tests/test_firmware_web.py`,
`test_wifi.py`.

## 8. Medición con una sola plantilla

**Por qué:** los canales FSR del pie derecho (`P2_H`, `P2_F`) no reciben señal en esta placa, y no
hubo medios para repararlos. Mientras tanto se usa una sola plantilla conectada a los canales
`P1_H` y `P1_F`, y se mide un pie por toma.

**Qué cambió en la interfaz** (solo frontend; el backend no se modificó y sigue calculando sus
campos bilaterales, que la interfaz ignora):
- Se quitó el paso "Verifica los sensores". Preparación queda en 3 pasos: conectar, tara y listo.
- Aparece el selector **Pie medido** (Izquierdo / Derecho) en el paso final de Preparación y en
  Evaluación. Se guarda en el navegador y se cambia entre tomas sin reconectar.
- Cada salto queda etiquetado con su pie (resultado, progreso y tabla del modo profesional). La
  etiqueta se guarda en el navegador, no en la base de datos.
- Se ocultan el reparto izquierda/derecha, la simetría de carga (LSI) y el aviso de asimetría, y
  la plantilla dibuja solo el pie medido. Hay un aviso visible: "Medición con una plantilla: sin
  comparación entre pies."

**Procedimiento de medición**
1. Conecta la plataforma (sección 5) y haz la tara.
2. Pon la plantilla del pie que vas a medir en el **mismo conector** de siempre (`P1`).
3. En el paso final, elige el pie medido y empieza la evaluación.
4. Haz 3 a 5 saltos con ese pie, siempre con la misma altura y el mismo protocolo.
5. Para medir el otro pie: cambia la plantilla al mismo conector, cambia el selector a ese pie y
   repite el mismo número de saltos.
6. Compara los **promedios** de cada pie, no un salto contra otro: dos saltos distintos no son
   comparables por sí solos.

**Límites que hay que conocer**
- No hay datos simultáneos de los dos pies: no se puede medir asimetría de carga ni diferencia de
  contacto entre pies en un mismo aterrizaje.
- Las reglas "Ambos pies a la vez" y "Peso repartido" se **ocultan en la interfaz** (resultado,
  acciones, fortalezas y tabla por ítem), pero el backend sigue calculándolas.
- **El índice de técnica (0-100) sí las incluye.** Con el pie derecho vacío, "Peso repartido" cuenta
  como medible y sale "no cumple" (pesa 10 puntos de ~100); "Ambos pies a la vez" pesa 5 más. Con la
  placa real el índice puede salir hasta unos 10-15 puntos más bajo de lo que le correspondería.
  Hasta corregirlo en el backend, interpreta el índice como orientativo y compara solo saltos medidos
  con esta misma configuración.
- La etiqueta del pie vive en el navegador: si cambias de navegador o borras sus datos, se pierde.

## 9. Compatibilidad entre sistemas

- La plataforma (`iniciar.sh`, `iniciar.ps1`, `.command` y `.cmd`) y la detección de puertos USB
  (`/dev/cu.*`, `/dev/ttyUSB*`, `COM*`) ya contemplaban macOS, Linux y Windows.
- Lo añadido para el WiFi usa solo funciones estándar de Python y de sockets (`socket://` de
  pyserial), sin rutas ni llamadas específicas de un sistema.
- Se corrigió `AuthUtil.h`, que usaba `strncasecmp` (POSIX, ausente en MSVC), y la prueba de C++ para
  que el ejecutable lleve `.exe` en Windows.
- Se probó en macOS (real) y en Linux (contenedor Python 3.12; ver resultado en la sección 10).
  **Windows no se pudo probar** en este trabajo: los scripts de Windows y los comandos de PowerShell de
  esta guía están escritos por revisión de código, no ejecutados.

## 10. Qué está verificado y qué no

**Linux (contenedor Debian con Python 3.12 y GCC 14):** las 34 pruebas del backend pasan, incluidas
las del WiFi y las de C++ del firmware. No se probó la interfaz web ni un puerto USB real en Linux.

**Verificado con la placa real:** carga y arranque del firmware 0.3.0; detección de las IMU (muslo
MPU6050 + QMC5883P, pierna GY-85 + HMC5883L, lumbar ausente); punto de acceso activo; acceso web
configurado; puertos 80 y 3333 aceptan conexión desde la Mac; conexión de la plataforma por WiFi y
llegada de datos; copia y lectura de la NVS.

**No verificado (requiere prueba física):** reconexión a una red WiFi externa; bloqueo por intentos
fallidos desde un navegador; aspecto del panel en el móvil; que las lecturas del panel coincidan
con el CSV; el comportamiento del flujo TCP con un cliente lento.

**Problema de hardware abierto:** los canales FSR `P2_H` y `P2_F` (GPIO 34 y 35) no reciben señal;
`P2_H` solo muestra diafonía. No se pudo medir con multímetro, por lo que la causa concreta no
está confirmada.

## 11. Lista de comprobación para Windows (y para repetir en Linux)

Para ejecutar en un equipo Windows 10/11. Marca cada punto y anota el resultado exacto si algo
falla (mensaje de error completo). Lo que ya está verificado en Mac y Linux es lo de la sección 10;
esta lista cubre lo que **no** se pudo probar en Windows.

**A. Instalación y arranque de la plataforma**
- [ ] `git clone` del repositorio y comprobar que `plataforma\iniciar.ps1` tiene finales de línea
  de Windows (CRLF) y los `.sh` se quedan en LF (lo fija `.gitattributes`).
- [ ] Abrir PowerShell en `plataforma` y ejecutar `.\iniciar.ps1`. Si bloquea la ejecución de
  scripts: `powershell -ExecutionPolicy Bypass -File .\iniciar.ps1`.
- [ ] Alternativa: doble clic en `Abrir Aterriza (Windows).cmd`.
- [ ] Se instalan `uv` y las dependencias (necesita internet la primera vez) y el navegador abre
  `http://127.0.0.1:8000`.
- [ ] La pantalla de inicio carga y se puede crear o elegir un atleta.
- [ ] Preparación → Gemelo (demostración) → Conectar → Tara → Empezar evaluación → un salto demo con
  su etiqueta de pie. Sin errores en la consola del navegador (F12).

**B. Pruebas automáticas (opcional, requiere un compilador de C++ tipo MinGW)**
- [ ] `cd plataforma\backend; uv run pytest -q`. Esperado: `34 passed`. Sin compilador, las dos
  pruebas de C++ se omiten (`skipped`) y el resto debe pasar.

**C. Puerto USB**
- [ ] Conectar la placa con un cable de datos. Aparece un puerto `COMx` en el Administrador de
  dispositivos (si no, instalar el driver CP210x o CH340).
- [ ] Preparación → ESP32 por USB: el puerto aparece en la lista y se elige solo.
- [ ] Conectar: llegan datos (la interfaz pasa a la tara) o aparece un mensaje de error claro.

**D. WiFi**
- [ ] Unir el equipo a la red `Aterriza-ESP32` y comprobar con `ipconfig` una IP `192.168.4.x`.
- [ ] `Test-NetConnection 192.168.4.1 -Port 80` y `-Port 3333` dan `TcpTestSucceeded : True`.
- [ ] `http://192.168.4.1/` abre el panel (o `/setup` la primera vez).
- [ ] Preparación → ESP32 por WiFi, host `192.168.4.1` → Conectar. Llegan datos y se puede tarar.
- [ ] Si Windows muestra el cortafuegos, permitir el acceso a la red privada para Python.

**E. Medición con una plantilla**
- [ ] Elegir "Pie medido", hacer un salto y comprobar que el resultado lleva la etiqueta del pie y
  el aviso de una plantilla.

**Qué anotar si algo falla:** el paso, el mensaje de error completo, la versión de Windows
(`winver`) y la salida de `uv --version` y `node --version`.
