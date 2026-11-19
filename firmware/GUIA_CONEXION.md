# Guía de uso de Aterriza (por USB)

Esta guía tiene dos partes:

- **Parte A, paso a paso (no necesitas saber programar).** Cómo encender el equipo, abrir la
  plataforma y medir un salto usando el cable USB. Empieza aquí.
- **Parte B, referencia técnica.** Para quien mantiene el equipo: puertos, calibración, copia de
  seguridad, carga del firmware, WiFi opcional y qué está verificado.

El WiFi es una vía **opcional** (Apéndice B6). Todo lo que necesitas para medir funciona con el
cable USB.

---

# PARTE A. Paso a paso para medir (USB)

## A1. Qué necesitas

- [ ] La placa del equipo (ESP32) con las dos IMU puestas (muslo y pierna) y la plantilla de carga.
- [ ] Un cable USB **de datos** (los cables que solo cargan no funcionan).
- [ ] Un computador con macOS, Windows o Linux.
- [ ] Internet **solo la primera vez**, para descargar lo que necesita el programa.
- [ ] Una persona que haga los saltos y otra que maneje el computador (recomendado).

## A2. Instalación (se hace una sola vez)

1. **Instala Node.js** (versión 20 o superior) desde https://nodejs.org (botón "LTS"). Acepta todas
   las opciones por defecto. En Windows también sirve abrir PowerShell y escribir
   `winget install OpenJS.NodeJS.LTS`.
2. **Descarga el proyecto.** En la página del repositorio pulsa *Code → Download ZIP* y descomprime
   la carpeta (por ejemplo en el Escritorio).
3. **Listo.** El resto (Python y las librerías) lo instala el propio programa la primera vez que lo
   abras. Ese primer arranque tarda unos minutos.

Si **Windows** pide un driver al conectar la placa, instala el de **CP210x** (Silicon Labs) o
**CH340** (WCH), según lo que diga el chip de la placa, y prueba con otro cable si no aparece.

## A3. Conectar la placa

1. Con el computador encendido, conecta la placa con el cable USB. No hace falta cargador aparte.
2. Colócate los sensores como de costumbre: una IMU en el muslo, otra en la pierna, bien sujetas con
   su correa, sin que se balanceen. La plantilla va dentro del zapato del pie que vas a medir.
3. Asegúrate de que el cable **no tire** de la placa cuando la persona se mueva. Si se desconecta, la
   medición se pierde. Fíjalo con cinta o a la ropa dejando holgura.

## A4. Abrir la plataforma

Abre la carpeta `plataforma` del proyecto y haz lo siguiente según tu equipo.

| Sistema | Qué hacer |
|---|---|
| **Mac** | Doble clic en **Abrir Aterriza.command**. La primera vez macOS puede decir que no se puede abrir: haz clic derecho sobre el archivo → **Abrir** → **Abrir**. |
| **Windows** | Doble clic en **Abrir Aterriza (Windows).cmd**. Si aparece "Windows protegió su PC", pulsa **Más información → Ejecutar de todas formas**. |
| **Linux** | Abre una terminal en la carpeta `plataforma` y escribe `./iniciar.sh`. |

Verás una ventana negra con texto. **No la cierres** mientras uses la plataforma: es el programa en
marcha. Después de unos segundos (la primera vez, unos minutos) se abre solo el navegador en
`http://127.0.0.1:8000`. Si no se abre, escribe esa dirección tú en el navegador.

## A5. Elegir al atleta

En la pantalla de inicio, elige un atleta de la lista o pulsa **+ Nuevo atleta**, completa los datos
y **Crear y continuar**.

## A6. Conectar el dispositivo (paso 1 de Preparación)

1. En la pantalla de Preparación elige **ESP32 por USB**.
2. Aparece una lista **Puerto USB**. Normalmente el programa deja elegido el correcto. Si hay varios,
   elige el que contenga `usbserial`, `ttyUSB`, `ttyACM` o `COM`.
3. Pulsa **Conectar**.
4. Cuando funciona, la pantalla pasa sola al paso 2 y el muñeco 3D se mueve al mover las piernas.

Si no conecta, mira la tabla de la sección A10.

## A7. La tara (paso 2 de Preparación)

La tara fija el "cero" de los ángulos: dice a la plataforma cuál es tu postura derecha.

1. La persona se pone **de pie y derecha**, con el pie medido sobre la plantilla y el peso apoyado en
   él.
2. **Se queda completamente quieta**: sin hablar, sin mover las piernas, sin tocar los cables.
3. Pulsa **Iniciar tara** y espera una cuenta atrás de unos segundos.
4. Al terminar pasa al paso 3. Si dice que no detectó quietud, repite con la persona más quieta o
   revisa que las correas no dejen los sensores flojos.

## A8. Medir los saltos (paso 3: ¡Todo listo!)

1. Elige el tipo de salto con el selector: aterrizaje desde cajón (de 30 a 40 cm) o salto vertical en
   el sitio.
2. En **Pie medido** elige **Izquierdo** o **Derecho**: el pie que tiene la plantilla puesta.
3. Pulsa **Empezar evaluación**.
4. Sigue lo que dice la pantalla ("Súbete al cajón", "¡Aterriza!"). Haz **3 a 5 saltos** con el mismo
   protocolo y la misma altura.
5. Tras cada salto verás el análisis, una repetición en cámara lenta y el resultado con la etiqueta
   del pie medido.

**Para medir el otro pie:** termina la serie de saltos, pasa la plantilla al otro zapato **conectada en el
mismo conector de la placa**, cambia **Pie medido** al otro pie y repite el mismo número de saltos.
Compara los **promedios** de cada pie, no un salto contra otro.

## A9. Terminar y apagar

1. Cierra la pestaña del navegador.
2. En la ventana negra pulsa **Ctrl + C** (o ciérrala).
3. Desconecta el cable USB.

## A10. Si algo falla

| Qué ves | Qué hacer |
|---|---|
| No aparece ningún puerto en la lista | Prueba otro cable USB (debe ser de datos) y otro puerto del computador. En Windows, instala el driver (A2). Después recarga la página. |
| Pulso **Conectar** y no pasa al paso 2 | Comprueba que la placa tenga luz, que ningún otro programa (Arduino, monitor serie) esté usando el puerto y vuelve a intentar. |
| "No se detectó quietud" en la tara | La persona debe estar totalmente inmóvil durante la cuenta atrás. Revisa que las IMU estén bien sujetas y que el cable no tire. Repite la tara. |
| El muñeco no se mueve o se congela | Se soltó el cable. Vuelve a conectarlo y pulsa **Conectar** otra vez. |
| La ventana negra dice "Falta Node.js" | Instala Node.js (A2) y vuelve a abrir. |
| macOS o Windows bloquea el archivo | Mira la instrucción de A4 (clic derecho → Abrir, o Más información → Ejecutar de todas formas). |
| El navegador no abre solo | Escribe `http://127.0.0.1:8000` en la barra de direcciones. |

Si nada de esto resuelve el problema, anota qué paso fallaba y el mensaje exacto en pantalla antes
de pedir ayuda.

## A11. Qué hacer y qué evitar

- Haz la tara **siempre con la persona ya vestida con los sensores** y de pie, no sobre la mesa.
- No desconectes el cable mientras mides.
- No pulses los comandos de calibración ni borres calibraciones sin saber qué hacen (están en la
  Parte B).
- Si aparece el aviso "Medición con una plantilla: sin comparación entre pies", es lo esperado: este
  equipo mide un pie por toma.
- El índice de técnica (0-100) es **orientativo** en esta configuración; mira más los detalles del
  salto que el número solo.

---

# PARTE B. Referencia técnica

## B1. Resumen de vías

| Vía | Para qué | Cómo |
|---|---|---|
| **USB (principal)** | Medir, cargar firmware, comandos, backup, diagnóstico | Cable de datos, puerto serie (tabla de B2), 921600 baudios |
| WiFi (opcional) | Panel web y flujo de datos sin cable de datos | Red `Aterriza-ESP32`, panel `http://192.168.4.1/`, flujo TCP `192.168.4.1:3333` (ver B6) |

La placa puede usar USB y WiFi a la vez.

## B2. Conexión por USB y comandos

1. Conecta la placa con un cable USB **de datos**.
2. Busca el puerto (en los ejemplos se escribe `PUERTO`; sustitúyelo por el tuyo):

   | Sistema | Cómo verlo | Ejemplo |
   |---|---|---|
   | macOS | `ls /dev/cu.*` | `/dev/cu.usbserial-0001` |
   | Linux | `ls /dev/ttyUSB* /dev/ttyACM*` | `/dev/ttyUSB0` |
   | Windows | Administrador de dispositivos → Puertos (COM y LPT), o `arduino-cli board list` | `COM3` |

   - **Linux:** si da "permission denied", añade tu usuario al grupo del puerto y vuelve a iniciar
     sesión: `sudo usermod -aG dialout $USER` (en algunas distribuciones el grupo es `uucp`).
   - **Windows:** si el puerto no aparece, instala el driver del conversor USB-serie de tu placa
     (CP210x o CH340/CH341) y prueba otro cable de datos.
   - En esta placa, en Mac, el puerto es `/dev/cu.usbserial-0001`.
3. Abre el monitor serie a **921600 baudios** (por ejemplo
   `arduino-cli monitor -p PUERTO -c baudrate=921600`). A 115200 se ve basura: es normal.
4. Comandos de una letra: `h` ayuda, `i` estado (IMU, WiFi, errores), `c` calibraciones,
   `s`/`x` iniciar/detener datos, `t` tara, `g` giroscopio, `a` acelerómetro, `m` magnetómetro,
   `r` borrar calibraciones de IMU, `k` olvidar red WiFi y usuario web.

**Un solo programa por puerto.** Si la plataforma (o un monitor serie) tiene abierto el puerto, otro
programa no puede leerlo. Desconecta la plataforma antes de usar el monitor serie.

Al abrir el puerto desde un programa, la placa puede **reiniciarse**. Al arrancar recalibra el
giroscopio (los 2 s de reposo) y guarda el resultado: déjala quieta en ese momento.

## B3. Calibración

- **Giroscopio (`g`)**: el bias del chip no depende de dónde esté puesto, pero cambia un poco con la
  temperatura. Conviene recalibrarlo **con los sensores ya puestos en la persona, quieta de pie**, tras
  un par de minutos de calentamiento. Si el firmware detecta movimiento responde
  `#WARN IMU ...: movimiento detectado (std ... dps), se conserva el bias anterior` y **no cambia nada**;
  repite con más quietud. Un cable USB que tira de la placa basta para que lo rechace.
- **Tara (`t`)**: fija el cero de los ángulos. La plataforma la hace desde Preparación con la persona de
  pie; no hace falta enviar `t` a mano.
- **Acelerómetro (`a`) y magnetómetro (`m`)**: en este equipo siguen **sin calibrar**. Si se calibran,
  hay que hacerlo con los sensores ya fijados al cuerpo, porque el resultado depende de cómo están
  montados.
- **Caso conocido en este equipo (IMU de la pierna, GY-85):** su giroscopio tiene un desfase propio
  anormalmente grande (en la calibración, aproximadamente `[-31, -453, 3,8] °/s`). Es repetible, así que
  tras calibrar las lecturas quedan en ≈ 0 (|g| < 1,2 °/s). Aun así es un valor muy fuera de lo normal en
  un ITG3200; si reaparecen picos de cientos de °/s con el sensor quieto, revisa o cambia el módulo.
- Ver el estado actual: comando `c`. Guarda esa salida: es la calibración en texto legible.

## B4. Copia de seguridad (antes de cargar firmware)

Solo lee la placa; no la modifica. La herramienta `esptool` viene con el core de ESP32 instalado por
Arduino (versión 5.3.1 en el equipo donde se probó; la carpeta lleva el número de versión que tengas):

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

## B5. Cargar el firmware (solo si hay que actualizarlo)

Desde la carpeta `firmware/` (igual en los tres sistemas; sustituye `PUERTO`):

```
arduino-cli compile --fqbn esp32:esp32:esp32 BiomecanicaESP32
arduino-cli upload -p PUERTO --fqbn esp32:esp32:esp32 BiomecanicaESP32
```

- Necesita el core `esp32:esp32` (probado con 4.0.0). No requiere librerías externas.
- `upload` **no compila**: sube lo último que haya en la caché. Si antes compilaste con otras
  opciones (por ejemplo `-DCFG_WIFI_ENABLED=0` para la simulación), subirías esa versión. Compila
  siempre justo antes de subir y comprueba que el comando `i` muestre `WiFi: punto de acceso` y no
  `WiFi: desactivado en esta compilacion`.
- Subir la aplicación no borra la calibración ni la configuración de red (viven en otra partición).
- Haz antes la copia de seguridad (B4).

## B6. Apéndice opcional: WiFi

Úsalo solo si necesitas medir sin cable de datos (el equipo funciona entonces con su cargador).

**Acceso a la placa.** La placa crea su propia red cuando no tiene otra configurada.

1. Une el computador a la red **`Aterriza-ESP32`**. La clave por defecto es pública (está en
   `config.h`); cámbiala en `wifi_secrets.h` si el equipo sale del laboratorio.
2. Comprueba que tu IP empiece por `192.168.4.`: macOS: Ajustes → WiFi → Detalles · Linux: `ip -4 addr` ·
   Windows: `ipconfig`.
3. Abre `http://192.168.4.1/` escribiendo `http://` completo (algunos navegadores lo cambian a
   `https://`, que no existe aquí).
4. La primera vez te lleva a `/setup`: define **usuario y contraseña de acceso** (mínimo 8 caracteres,
   distintos de los del WiFi). La red WiFi del laboratorio es opcional (solo 2,4 GHz).
5. A partir de ahí el panel y `/api/status` piden usuario y contraseña. Tras 5 intentos fallidos se
   bloquea 30 s.
6. Si olvidas la contraseña: comando `k` por USB (olvida red y credenciales web; no toca la calibración).

**Importante:** mientras estés en `Aterriza-ESP32` no hay internet. Si necesitas internet a la vez,
comparte la conexión del móvil **por cable USB** o trabaja por turnos. El nombre `aterriza.local`
depende de mDNS (en Windows suele requerir Bonjour): usa siempre la IP `192.168.4.1`.

**Usar la plataforma por WiFi.** Con el equipo conectado a `Aterriza-ESP32`: Preparación → **ESP32 por
WiFi** → host **`192.168.4.1`** → Conectar. Por seguridad, el flujo de datos TCP (:3333) no pide
contraseña: úsalo solo en el punto de acceso propio o en una red de confianza.

**Diagnóstico del WiFi**

| Síntoma | Causa | Solución |
|---|---|---|
| `nodename nor servname provided` | Host `aterriza.local` sin resolver | Usar `192.168.4.1` |
| `Network is unreachable` | El equipo no estaba en `Aterriza-ESP32` (su IP era la de otra red) | Unir el equipo a esa red y revisar que la IP sea `192.168.4.x` |
| Conecta pero no llegan datos | `availableForWrite()` de `WiFiClient` devuelve 0 en este core y se descartaban los datos | Corregido en `WifiLink.cpp` con envío no bloqueante |
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

## B7. Medición con una sola plantilla

**Por qué:** los canales FSR del pie derecho (`P2_H`, `P2_F`) no reciben señal en esta placa, y no hubo
medios para repararlos. Mientras tanto se usa una sola plantilla conectada a los canales `P1_H` y
`P1_F`, y se mide un pie por toma. El procedimiento paso a paso está en A8.

**Qué cambió en la interfaz** (solo frontend; el backend no se modificó y sigue calculando sus campos
bilaterales, que la interfaz ignora):
- Se quitó el paso "Verifica los sensores". Preparación queda en 3 pasos: conectar, tara y listo.
- Aparece el selector **Pie medido** (Izquierdo / Derecho) en el paso final de Preparación y en
  Evaluación. Se guarda en el navegador y se cambia entre tomas sin reconectar.
- Cada salto queda etiquetado con su pie (resultado, progreso y tabla del modo profesional). La
  etiqueta se guarda en el navegador, no en la base de datos.
- Se ocultan el reparto izquierda/derecha, la simetría de carga (LSI) y el aviso de asimetría, y la
  plantilla dibuja solo el pie medido. Hay un aviso visible: "Medición con una plantilla: sin
  comparación entre pies."

**Límites que hay que conocer**
- No hay datos simultáneos de los dos pies: no se puede medir asimetría de carga ni diferencia de
  contacto entre pies en un mismo aterrizaje.
- Las reglas "Ambos pies a la vez" y "Peso repartido" se **ocultan en la interfaz** (resultado,
  acciones, fortalezas y tabla por ítem), pero el backend sigue calculándolas.
- **El índice de técnica (0-100) sí las incluye.** Con el pie derecho vacío, "Peso repartido" cuenta
  como medible y sale "no cumple" (pesa 10 puntos de ~100); "Ambos pies a la vez" pesa 5 más. Con la
  placa real el índice puede salir hasta unos 10-15 puntos más bajo de lo que le correspondería. Hasta
  corregirlo en el backend, interpreta el índice como orientativo y compara solo saltos medidos con
  esta misma configuración.
- La etiqueta del pie vive en el navegador: si cambias de navegador o borras sus datos, se pierde.

## B8. Cambios realizados en el proyecto

**Firmware** (`firmware/BiomecanicaESP32/`): `WifiLink.*` (conexión, reconexión, punto de acceso,
servidor TCP), `NetConfig.*` (red y credenciales en NVS, hash con sal), `WebNode.*` y `Dashboard.h`
(panel web autenticado), `StatusJson.*` y `Snapshot.h` (JSON de lecturas), `AuthUtil.h`, `Sha256.h`,
`App.*` (instantánea de lecturas y comando `k`), `config.h`, `wifi_secrets.example.h`. Detalle y
seguridad: `firmware/README.md`.

**Plataforma:** fuente "ESP32 por WiFi" (`laboratorio/core/link.py`, `plataforma/backend/app/bridge.py` y
`main.py`), pantalla de Preparación y barra superior del frontend.

**Pruebas:** `firmware/tests/host_test.cpp`, `plataforma/backend/tests/test_firmware_web.py` y
`test_wifi.py`.

## B9. Compatibilidad entre sistemas

- La plataforma (`iniciar.sh`, `iniciar.ps1`, `.command` y `.cmd`) y la detección de puertos USB
  (`/dev/cu.*`, `/dev/ttyUSB*`, `COM*`) contemplan macOS, Linux y Windows.
- Lo añadido para el WiFi usa solo funciones estándar de Python y de sockets (`socket://` de pyserial).
- Se corrigió `AuthUtil.h`, que usaba `strncasecmp` (POSIX, ausente en MSVC), y la prueba de C++ para que
  el ejecutable lleve `.exe` en Windows.
- Se probó en macOS (real) y en Linux (contenedor Python 3.12). **Windows no se pudo probar**: los scripts
  de Windows y los comandos de PowerShell de esta guía están escritos por revisión de código, no
  ejecutados.

## B10. Qué está verificado y qué no

**Verificado con la placa real (macOS):**
- Carga y arranque del firmware 0.3.0; detección de las IMU (muslo MPU6050 + QMC5883P, pierna GY-85 +
  HMC5883L, lumbar ausente).
- **Por USB:** la plataforma conecta a 100 Hz sin pérdidas; la tara detecta quietud y se completa;
  recalibración del giroscopio con los sensores puestos en la persona.
- Por WiFi: punto de acceso activo, acceso web configurado, puertos 80 y 3333 aceptan conexión y la
  plataforma recibe datos.
- Copia y lectura de la NVS.

**Linux (contenedor Debian con Python 3.12 y GCC 14):** las 34 pruebas del backend pasan, incluidas las
del WiFi y las de C++ del firmware. No se probó la interfaz web ni un puerto USB real en Linux.

**No verificado (requiere prueba física):**
- Un salto completo con la placa real (aterrizaje desde cajón de 35 cm con el pie medido y su etiqueta).
- Calibración de acelerómetro y magnetómetro.
- Reconexión a una red WiFi externa; bloqueo por intentos fallidos desde un navegador; aspecto del
  panel en el móvil; el comportamiento del flujo TCP con un cliente lento.
- Windows, y la barra superior de la interfaz en pantallas de móvil (se desborda a 390 px; puede ser
  anterior a estos cambios).

**Problemas de hardware abiertos:**
- Canales FSR `P2_H` y `P2_F` (GPIO 34 y 35) sin señal; `P2_H` solo muestra diafonía. No se pudo medir
  con multímetro, por lo que la causa concreta no está confirmada.
- Giroscopio de la IMU de la pierna con desfase muy grande (ver B3).

## B11. Lista de comprobación para Windows (y para repetir en Linux)

Para ejecutar en un equipo Windows 10/11. Marca cada punto y anota el resultado exacto si algo falla
(mensaje de error completo). Esta lista cubre lo que **no** se pudo probar en Windows.

**A. Instalación y arranque de la plataforma**
- [ ] `git clone` del repositorio y comprobar que `plataforma\iniciar.ps1` tiene finales de línea de
  Windows (CRLF) y los `.sh` se quedan en LF (lo fija `.gitattributes`).
- [ ] Abrir PowerShell en `plataforma` y ejecutar `.\iniciar.ps1`. Si bloquea la ejecución de scripts:
  `powershell -ExecutionPolicy Bypass -File .\iniciar.ps1`.
- [ ] Alternativa: doble clic en `Abrir Aterriza (Windows).cmd`.
- [ ] Se instalan `uv` y las dependencias (necesita internet la primera vez) y el navegador abre
  `http://127.0.0.1:8000`.
- [ ] La pantalla de inicio carga y se puede crear o elegir un atleta.
- [ ] Preparación → Demostración → Conectar → Tara → Empezar evaluación → un salto demo con su etiqueta
  de pie. Sin errores en la consola del navegador (F12).

**B. Pruebas automáticas (opcional, requiere un compilador de C++ tipo MinGW)**
- [ ] `cd plataforma\backend; uv run pytest -q`. Esperado: `34 passed`. Sin compilador, las dos pruebas
  de C++ se omiten (`skipped`) y el resto debe pasar.

**C. Puerto USB (vía principal)**
- [ ] Conectar la placa con un cable de datos. Aparece un puerto `COMx` en el Administrador de
  dispositivos (si no, instalar el driver CP210x o CH340).
- [ ] Preparación → ESP32 por USB: el puerto aparece en la lista y se elige solo.
- [ ] Conectar: llegan datos (la interfaz pasa a la tara) o aparece un mensaje de error claro.
- [ ] Tara con la persona quieta: se completa y pasa a "¡Todo listo!".

**D. WiFi (opcional)**
- [ ] Unir el equipo a la red `Aterriza-ESP32` y comprobar con `ipconfig` una IP `192.168.4.x`.
- [ ] `Test-NetConnection 192.168.4.1 -Port 80` y `-Port 3333` dan `TcpTestSucceeded : True`.
- [ ] `http://192.168.4.1/` abre el panel (o `/setup` la primera vez).
- [ ] Preparación → ESP32 por WiFi, host `192.168.4.1` → Conectar. Llegan datos y se puede tarar.
- [ ] Si Windows muestra el cortafuegos, permitir el acceso a la red privada para Python.

**E. Medición con una plantilla**
- [ ] Elegir "Pie medido", hacer un salto y comprobar que el resultado lleva la etiqueta del pie y el
  aviso de una plantilla.

**Qué anotar si algo falla:** el paso, el mensaje de error completo, la versión de Windows (`winver`) y
la salida de `uv --version` y `node --version`.
