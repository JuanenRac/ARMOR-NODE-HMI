<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="Banner de ARMOR-NODE-HMI" width="100%">
</p>

# 🖥️ ARMOR-NODE-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  🇪🇸 <b>Español</b> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### El panel táctil de la casa: una pantalla de 7 pulgadas en la pared que muestra el estado del sistema, lo arma y desarma y reconoce alarmas, con micrófono y altavoz para el asistente de voz que vendrá (un nodo de la familia en la ESP32-S3-Touch-LCD-7C-BOX, con su propia página web de ajustes)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 o posterior">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Lenguaje">
  <img src="https://img.shields.io/badge/Board-ESP32--S3--Touch--LCD--7C--BOX-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Checks-1%2C759-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-93122%20checks-2ea44f.svg" alt="Pruebas">
  <img src="https://img.shields.io/badge/Version-0.4.2-0f6cbd.svg" alt="Versión">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Madurez">
</p>

---

**Comprobación de honestidad - qué funciona hoy:** Lo que no depende de la placa es real y está probado en un ordenador (los ajustes, lo que el panel sabe del servidor y cuándo está desfasado, la alarma de coacción que nunca se muestra, la conversación de voz y la respuesta larga del servicio de voz, los pines de la placa, los textos de la pantalla en ocho idiomas y los glifos de sus fuentes, las comprobaciones de versión y firma: 30,783 comprobaciones en 19 programas de test) y también lo está la página web (ejecutada contra un sustituto en un navegador real). **El firmware se ha compilado una vez (ESP-IDF 5.5.5, que la integración continua no comprueba) y nunca ha corrido en una placa:** la pantalla, el táctil, el sonido y el enlace con el servidor están escritos a partir de los ejemplos de Waveshare para esta familia de placas y necesitan su primer encendido. El servicio de voz (ARMOR-VOICE-AI) atiende las peticiones de este panel; no hay palabra de activación: es pulsar para hablar.

---

## 📋 De un vistazo

| | |
|---|---|
| Versión | `0.4.2` |
| Madurez | Funcional |
| Función | Panel táctil de pared para ARMOR-SERVER (familia: Clients, padre: ARMOR-SERVER) |
| Tecnología | C++17, ESP-IDF 5.5.5, LVGL, ESP32-S3 |
| Pruebas | 93122 comprobaciones en 19 programas de host y 6 de Python (`ctest`) |
| Placa | Waveshare ESP32-S3-Touch-LCD-7C-BOX (800x480, GT911) |
| Funciona en | Una Waveshare ESP32-S3-Touch-LCD-7C-BOX; pruebas de host en cualquier ordenador |
| En hardware real | Ninguno todavía: el firmware compila (ESP-IDF 5.5.5, sin comprobar en CI) pero no ha corrido en la placa |
| Licencia | GPL-3.0-or-later |

**Ir a:** [Resumen](#-resumen) · [Arquitectura](#-arquitectura) · [Seguridad](#-seguridad-y-protección) · [Interfaces](#-interfaces) · [Configuración](#-configuración) · [Inicio rápido](#-inicio-rápido) · [Pruebas](#-pruebas) · [Problemas](#-resolución-de-problemas) · [Estructura](#-estructura-del-repositorio) · [Relacionados](#-proyectos-relacionados)

## 🚦 Estado del proyecto

Cada línea es una parte de este repositorio y la luz dice hasta dónde llega hoy. Está escrito con lo que el repositorio puede demostrar (pruebas, compilaciones, ejecuciones), no con lo que está planeado.

🟢 Hecho y verificado · 🟡 Funciona, con límites conocidos · 🟠 Parcial o sin verificar todavía · 🔴 No hecho o no funciona

| Área | Estado | Qué significa aquí |
|---|---|---|
| Software | 🟠 | El firmware se ha compilado una vez (ESP-IDF 5.5.5), pero la compilación no la comprueba la integración continua y nunca ha funcionado en una placa. |
| Pruebas | 🟢 | Pasan 93.122 comprobaciones en 19 programas de host (`ctest`): los ajustes, lo que el panel sabe del servidor y cuándo está obsoleto, la alarma de coacción que nunca se muestra, el intercambio de voz, los pines, los textos de la pantalla en ocho idiomas y los glifos de sus fuentes. |
| Protocolos y datos | 🟡 | El intercambio con el servidor y con el servicio de voz ARMOR-VOICE-AI se prueba contra simulaciones; es pulsar para hablar, sin palabra de activación. |
| Interfaz | 🟠 | La página web se ha ejecutado contra una simulación en un navegador real; la pantalla, el táctil y el sonido están escritos a partir de los ejemplos de Waveshare y esperan su primer encendido. |
| Hardware | 🔴 | El firmware no ha funcionado en la placa: nada ha funcionado todavía con equipos reales. |
| Seguridad | 🟡 | El código de configuración, el límite de accesos, el PIN Bluetooth, el manejo de la coacción y las actualizaciones firmadas están probados en el host; no ha habido una revisión independiente. |
| Actualizaciones | 🟠 | Las comprobaciones de hash y de firma pasan en el host; una actualización por aire nunca se ha ejecutado en una placa. |
| Integración | 🟠 | Los mensajes se validan contra los esquemas compartidos de ARMOR-COMMON; el enlace con el servidor y con el servicio de voz no se ha probado desde una placa. Su cliente MQTT se prueba contra un Mosquitto real en los tests. |
| Documentación | 🟢 | Ocho idiomas, la guía del firmware, las notas de cableado y el historial de cambios están al día. |

## 🎯 Resumen

* **Cliente de ARMOR-SERVER, como la app de Android:** entra con un usuario que un administrador creó para él, pide `GET /api/v1/panel/summary` (unos cientos de bytes: el modo, los nodos conectados y las alarmas que necesitan a una persona) cada pocos segundos y, cuando la persona toca el botón y confirma, arma, desarma o reconoce. Si el servidor deja de responder, la pantalla avisa de que lo que muestra es antiguo y el botón se desactiva. Escucha el canal en directo del servidor (un WebSocket) para que un cambio se vea al instante, y pregunta como antes cuando el canal falta, calla o está desactivado.
* **La pantalla (800x480, LVGL):** el modo en el color de la peor alarma que nadie ha visto, un botón grande de armar/desarmar con pregunta previa, los nodos conectados, las alarmas (las no vistas primero), una noche que la atenúa, un reposo tras un tiempo sin toques, un aviso sonoro y que se despierte cuando llega una alarma nueva, y ocho idiomas con acentos, japonés y chino dibujados con fuentes generadas justo para los caracteres usados.
* **Un nodo como los demás:** el mismo almacén de ajustes, Wi-Fi, página web en ocho idiomas, configuración por Bluetooth desde el móvil, actualización de firmware con vuelta atrás y HTTPS que ARMOR-NODE-RADAR, ARMOR-NODE-SOLAR y ARMOR-NODE-ELECTRICAL (comparten `ARMOR-COMMON/firmware_base`), más páginas para el servidor, la pantalla, el sonido y la voz; avisa a Studio de que existe por MQTT para que lo liste con su página.
* **El asistente de voz, pulsar para hablar:** un toque abre el micrófono, el audio va solo al servicio de voz cuya dirección está en los ajustes (apagado por defecto), la respuesta se muestra y se dice; el orden de los pasos, los límites de tiempo y lo que hace un toque son una máquina de estados probada, y el panel no ejecuta ninguna orden por sí mismo ([el contrato](docs/VOICE.md)). La grabación va como bytes en una petición binaria y la respuesta hablada vuelve como bytes; a un servidor que no la acepta se le envía la petición antigua.
* **La placa:** Waveshare ESP32-S3-Touch-LCD-7C-BOX (32 MB de flash, 16 MB de PSRAM, pantalla RGB de 7 pulgadas con tacto GT911, cuatro micrófonos, un altavoz); los pines vienen de los ejemplos de Waveshare y una prueba demuestra que dos funciones no comparten un GPIO ([la placa](docs/HARDWARE.md)).
* **Actualizaciones por aire firmadas:** el nodo solo instala desde GitHub una imagen cuyo SHA-256 coincide y que lleva una firma Ed25519 válida del responsable (la clave pública está en el firmware, la privada nunca sale del responsable); un archivo subido a mano por un administrador en el panel se sigue aceptando. Las páginas y mensajes del panel están en ocho idiomas.
* **Emparejamiento Bluetooth con PIN:** el PIN de seis cifras se deriva del código de configuración, se imprime en la consola USB y se muestra a un administrador en el panel; el móvil tiene que escribirlo antes de poder cambiar los ajustes del nodo.
* **Todavía no:** una primera compilación y encendido del firmware, una palabra de activación, el servicio de voz en la Jetson y una vista de las cámaras en la pantalla.
* **Solo la casa puede usar el panel:** el panel responde por defecto a la red privada (o a las redes que su dueño indique, como una VPN), rechaza una lista que dejaría fuera a quien la guarda, cuenta las contraseñas erróneas por persona y por dirección, y funciona detrás del proxy del servidor para llegar a él desde fuera de casa sin publicar el nodo.
* **Dice qué pasa:** la pantalla conoce el texto, en ocho idiomas, de cada código de alarma que levanta el servidor (red, solar, eléctrica, zonas de alarma...), y avisa al servidor de que es un panel de pared, así que se lista como tal y no cuenta como radar.
* **Enlace cifrado opcional con el broker (TLS):** desactivado por defecto, así que el nodo habla MQTT sin cifrar como antes. Un interruptor y una caja para el certificado de la autoridad propia de la instalación (PEM, 2 KB como máximo) en la página Broker hacen que se conecte con `mqtts://`, confíe solo en esa autoridad y compruebe que el certificado del broker se hizo para la dirección a la que llegó; lo que no sea un único certificado PEM se rechaza. Cubierto con pruebas en el ordenador; el handshake aún no se ha ejecutado en una placa.
* **Informe de salud:** cada minuto más o menos el nodo publica cuánto lleva encendido y por qué se reinició la última vez, su memoria libre, su enlace y su señal Wi-Fi, cuántas veces se conectó al broker y el último error que anotó (`armor/health/<nodo>`, el contrato `node_health` de ARMOR-COMMON). El código que lo construye lo comparten todos los nodos y está probado en un ordenador; no se ha ejecutado en una placa.
* **Control desde el servidor:** Studio puede reiniciar el nodo limpiamente, pausarlo hasta una hora (se reanuda solo), poner su trabajo en espera (sin fin, se conserva al reiniciar, el nodo sigue conectado y respondiendo), hacer que se muestre, pedirle una instantánea de diagnóstico y marcarlo en mantenimiento (`armor/control/<nodo>/...`, el contrato `node_control` de ARMOR-COMMON). Cada orden son dos pasos con un token de un solo uso que pertenece a esa operación, solo el servidor puede escribirlas en el broker, y no existe restablecimiento de fábrica, borrado ni sueño profundo. Mientras el panel está en pausa o en espera la pantalla muestra una pantalla propia que lo dice y no atiende toques; cuando se le pide que se identifique, parpadea una barra arriba (nunca un sonido). El código es compartido por todos los nodos y se prueba en un ordenador; no ha funcionado en una placa.
* **Diagnóstico sin cable:** tras un arranque que no fue limpio (un cuelgue, un watchdog, una caída de la alimentación, una reserva de memoria fallida) el informe de salud dice el motivo durante diez minutos; un fallo deja su volcado de núcleo (core dump) para verlo, descargarlo o borrarlo desde el panel; los ajustes guardados se copian antes de cada instalación de firmware y se recuperan si una actualización los daña; y un administrador puede pedir al nodo, a través del servidor, un paquete de diagnóstico o su registro con un token de un solo uso.
* **Una sirena que se repite y suena por orden:** todo sonido pasa por una cola con prioridades (sirena, alarma, voz, confirmación, aviso); la sirena se repite mientras haya alarmas críticas sin reconocer (`audio.siren_repeat_s` 30, `audio.siren_max_repeats` 10) y las alarmas mantienen un volumen mínimo (`audio.critical_min_volume` 40); la voz cede ante una alarma.
* **Un reloj con batería:** el chip PCF85063A de la placa da la hora al arrancar cuando el nodo aún no tiene hora real y el chip es de fiar, y se corrige cada vez que se sincroniza o se pone la hora (`time.rtc_enabled`, activado por defecto).
* **El estado de una orden, en la pantalla:** el pie dice que la orden se está enviando mientras va de camino y que se confirmó durante unos segundos cuando se cumplió; un rechazo conserva ahí su motivo, como antes.
* **Sellado opcional de los secretos guardados:** desactivado por defecto (`security.seal_secrets`, o `nodes.nvs_encryption` del servidor). Activado, las contraseñas, tokens, claves, PIN y hashes de contraseña que guarda el nodo quedan sellados con ChaCha20-Poly1305 y todo lo demás sigue legible; activarlo o desactivarlo los reescribe sin perder nada, aunque el nodo se pare a medias. No se quema ningún eFuse ni se cifra la flash: la clave está en una partición del mismo chip, así que **no** protege frente a quien tenga la placa, y un nodo instalado antes de esa partición conserva sus secretos como estaban hasta que se grabe con la nueva tabla de particiones.

## 🧩 Papel en A.R.M.O.R.

ARMOR-NODE-HMI es el panel de pared: un cliente de [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), como la app Android, con una pantalla de 7 pulgadas, micrófono y altavoz. Muestra el modo, los nodos en línea y las alarmas que necesitan a una persona, arma y desarma (con el PIN) y reconoce alarmas. Es donde vivirá el asistente de voz ([ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)) cuando exista su servicio de voz y, como todo nodo, se configura y actualiza desde [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO).

| Nombre | Cómo | Para qué |
|---|---|---|
| ARMOR-SERVER | HTTPS `GET /api/v1/panel/summary`, WebSocket `/api/v1/panel/events` | El resumen de la casa (modos, alarmas, nodos, energía) y sus cambios; el servidor decide qué hace una orden |
| ARMOR-VOICE-AI | a través del servidor | El audio de un turno va al servicio de voz; la respuesta se muestra y se dice |
| ARMOR-STUDIO | menú Panel táctil | Muestra la salud del panel y abre su panel web |
| ARMOR-COMMON | base de firmware | El firmware común (panel, accesos, actualización firmada, puesta en marcha por Bluetooth) se copia de él |

## 🏛️ Arquitectura

<p align="center">
  <img src="images/architecture.svg" alt="Arquitectura de ARMOR-NODE-HMI: quién lo llama, qué contiene y de qué depende" width="100%">
</p>

## 🖼️ Capturas de pantalla

Capturadas a 1920×1080 en un banco de pruebas, con datos de ejemplo y sin ninguna instalación real detrás.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Resumen: el estado del nodo de un vistazo - red, broker, firmware y lo que reporta." width="100%"><br><sub>Resumen: el estado del nodo de un vistazo - red, broker, firmware y lo que reporta.</sub></td><td width="50%" valign="top"><img src="images/screenshots/screen.png" alt="Pantalla y sonido: brillo, horas nocturnas, volumen y qué alarmas se anuncian." width="100%"><br><sub>Pantalla y sonido: brillo, horas nocturnas, volumen y qué alarmas se anuncian.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/voice.png" alt="Asistente de voz: pulsar para hablar, desactivado por defecto, y el servicio al que va su audio." width="100%"><br><sub>Asistente de voz: pulsar para hablar, desactivado por defecto, y el servicio al que va su audio.</sub></td><td width="50%" valign="top"><img src="images/screenshots/server.png" alt="Servidor: la dirección del servidor A.R.M.O.R., el usuario del panel y cada cuánto pregunta." width="100%"><br><sub>Servidor: la dirección del servidor A.R.M.O.R., el usuario del panel y cada cuánto pregunta.</sub></td></tr>
</table>

## 🔒 Seguridad y protección

| Amenaza | Defensa |
|---|---|
| Contraseñas en claro o compiladas | PBKDF2-HMAC-SHA256 con 10 000 rondas; ninguna contraseña en el firmware (un nodo sin usuario entra en puesta en marcha); máximo 4 usuarios (admin o visor) y de 8 a 64 caracteres; máximo 6 sesiones, 30 minutos de inactividad |
| Tomar un nodo recién grabado | La puesta en marcha exige un código: el de la compilación (`CONFIG_ARMOR_SETUP_CODE`) o uno aleatorio guardado en flash y mostrado en la consola USB; con secreto de flota es `HMAC-SHA256(secreto, MAC)`; también es la clave del Wi-Fi de configuración (WPA2, nunca puenteado) |
| El panel alcanzado desde internet | Responde por defecto a las redes privadas de una casa (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 y 100.64/10 donde viven algunas VPN); el ajuste `web.admin_sources` lista otras direcciones y redes IPv4 (8 entradas, 120 caracteres); lo de fuera se rechaza salvo la identidad pública `GET /api/v1/session`; se rechaza una lista que dejaría fuera a quien la guarda |
| El panel en claro por HTTP | HTTPS en el 443 con un certificado que el nodo hace él mismo (autofirmado, en flash, huella SHA-256 visible para comparar); el panel puede ser http, https o ambos; la cookie de sesión es siempre HttpOnly y SameSite=Strict (y Secure por TLS) |
| Un firmware falso desde GitHub | Una imagen solo se instala con un SHA-256 que coincide y una firma Ed25519 válida hecha con la clave del mantenedor (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` la rechazan); una subida manual de un administrador no se comprueba: es un límite explícito |
| Secuestro de la puesta en marcha por Bluetooth | Un PIN de 6 dígitos derivado del código de puesta en marcha, visible en la consola USB y para el administrador; el móvil debe introducirlo antes de escribir ajustes; la ventana dura `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 por defecto, de 1 a 120) |
| Secretos guardados leídos por quien tiene la placa | Opcional (apagado por defecto, interruptor `security.seal_secrets` o `nodes.nvs_encryption` del servidor): ChaCha20-Poly1305 para contraseñas, tokens, claves, PIN y hashes. No cifra la flash ni quema un eFuse: la clave está en una partición del mismo chip, así que no protege frente a quien tenga la placa |
| El enlace con el broker en claro | TLS opcional (`mqtts://`) con la autoridad propia del dueño (PEM de hasta 2 KB) y comprobación del nombre; apagado por defecto. El handshake solo se ha ejecutado en pruebas de host, no en una placa |
| La alarma de coacción mostrada en pantalla | Nunca se muestra (una prueba de host lo comprueba); y lo que la central sabe del servidor se marca como caducado cuando es viejo |
| Un servidor que no es el servidor | El certificado del servidor está fijado (`server_pin`), para las peticiones y para el WebSocket del canal vivo; la central entra con su propio usuario, creado por un administrador, con el rol `panel` (no puede usar Studio) |
| Desarmar sin PIN | Armar y desarmar preguntan antes y piden el PIN en pantalla (`pin_entry`); el servidor lo vuelve a comprobar |
| Audio de voz a un lugar inesperado | Apagada por defecto (`voice.enabled` falso); el audio va solo al servicio cuya dirección está en los ajustes (la dirección se valida y cambiar de host borra el token); la central no ejecuta ninguna orden por sí sola y no hay palabra de activación: es pulsar para hablar |
| Una sirena que nadie oye | Una sola cola de prioridad (sirena, alarma, voz, confirmación, aviso); la sirena se repite mientras haya alarmas críticas sin reconocer (`audio.siren_repeat_s` 30, de 10 a 600, 0 = sin repeticiones; `audio.siren_max_repeats` 10) y el volumen crítico nunca baja de `audio.critical_min_volume` (40 %) |

El firmware se compiló una vez y nunca ha corrido en una placa: la pantalla, el táctil, el sonido y el enlace con el servidor están escritos a partir de los ejemplos de Waveshare, a la espera del primer encendido. Lee [SECURITY.md](SECURITY.md) para informar de una vulnerabilidad.

## 🔌 Interfaces

| Parte | Qué define |
|---|---|
| ARMOR-SERVER | `GET /api/v1/panel/summary` cada `server.poll_s` (3 s, de 1 a 60) solo mientras el canal vivo está caído, y el WebSocket `/api/v1/panel/events` mientras `server.live_events` sea verdadero (por defecto): el resumen llega cuando cambian el modo, las alarmas, los nodos o la energía; el usuario propio de la central tiene el rol `panel` |
| Voz | El audio de un turno va a ARMOR-VOICE-AI a través del servidor (`/api/v1/voice/turn`, con cuerpo multipart opcional); `core/voice_session.hpp` es la máquina de estados (orden de los pasos, límites de tiempo, qué hace un toque); mira [docs/VOICE.md](docs/VOICE.md) |
| Hardware | Waveshare ESP32-S3-Touch-LCD-7C-BOX: pantalla RGB de 7 pulgadas 800x480, táctil GT911, cuatro micrófonos, un altavoz, RTC PCF85063A (`time.rtc_enabled`, verdadero por defecto), 32 MB de flash y 16 MB de PSRAM; una prueba demuestra que ninguna función comparte un GPIO |

## ⚙️ Configuración

| Variable | Por defecto | Qué define |
|---|---|---|
| `server.poll_s`, `server.live_events` | 3 (1-60), true | El periodo de sondeo del resumen y el canal vivo por WebSocket |
| `display.sleep_s` | 300 (0-3600) | Segundos hasta que la pantalla duerme (0 = nunca); el brillo y el brillo nocturno también son ajustes |
| `audio.critical_min_volume`, `audio.siren_repeat_s`, `audio.siren_max_repeats` | 40 (0-100), 30, 10 | El volumen mínimo de un sonido crítico, el periodo de repetición de la sirena (0 o de 10 a 600 s) y sus repeticiones máximas |
| `audio.tone_by_source` | según la gravedad | El sonido de cada origen de alarma (dispositivo, solar, eléctrico, red, central, otro): el que dice su gravedad, como aviso, como alarma, como sirena sin sonido, `spoken` (la alarma dicha en voz alta) o `custom` (el sonido subido para ese origen), ambos hechos por el servidor y reproducidos solo si llegan enteros, y si no el tono habitual; un sonido elegido suena aunque la gravedad esté por debajo del mínimo |
| `audio.clip_max_bytes` | 200000 | El máximo de audio, en bytes de 16 kHz, 16 bits, mono (de 3200 a 262144), que se toma del servidor para una alarma dicha en voz alta o un sonido propio; una respuesta más larga no se reproduce |
| `voice.enabled`, `voice.url`, `voice.token` | false, "", - | El servicio de voz: apagado por defecto; la dirección tiene como máximo 160 caracteres y se valida (`required` o `invalid` si no); el token se borra al cambiar de host |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | Los ajustes que comparten todos los nodos: el nombre (vacío da `armor-` y los últimos bytes de la MAC), el código de puesta en marcha (vacío: aleatorio), el secreto de flota (16+ caracteres, nunca en el repositorio), los primeros ajustes del broker, el latido (de 2 a 300 s), los minutos de puesta en marcha por Bluetooth (de 1 a 120) y el reinicio con BOOT (8 s en los primeros 30 s) |

## 🚀 Inicio rápido

**Requisitos:** un compilador C++17 para las pruebas de host; el contenedor ESP-IDF 5.5.5 para compilar la imagen; una Waveshare ESP32-S3-Touch-LCD-7C-BOX.

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel of the node without a board
tools/build_node.sh                                   # the image in the ESP-IDF container
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

Mira el [diseño](docs/DESIGN.md), la [placa](docs/HARDWARE.md), el [asistente de voz](docs/VOICE.md), la [guía del firmware](docs/NODE_FIRMWARE.md) y las [fuentes](docs/FONTS.md).

Para compilar el firmware a mano en GitHub Actions, mira [compilar en GitHub Actions](docs/FIRMWARE_BUILD_ACTIONS.md).

## 🧪 Pruebas

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 programas de prueba de host y 6 comprobaciones en Python se ejecutan con `ctest`; 93122 comprobaciones contadas por los programas de host, compilados con `-Werror`.

Comprobaciones Python: los textos del panel en ocho idiomas, las tareas, las rutas de guardado, la sesión Bluetooth bajo su bloqueo y las rutas que llama el panel (y, en algunos nodos, los códigos HTTP o los glifos de las fuentes)

Las pruebas de host necesitan cualquier compilador C++17 (Linux, WSL, MSYS2); el panel web también se ejercita en un navegador real contra un nodo de pruebas (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

`check_font_glyphs.py` comprueba que las fuentes tienen todos los glifos de los ocho idiomas (98 filas); `test_clock` solo imprime `ok`.

## 🏗️ Compilación y publicación

`tools/build_node.sh` compila la imagen en el contenedor ESP-IDF 5.5.5 y la versión lleva la imagen, su SHA-256 y su firma Ed25519, una por placa; la imagen se graba por USB-C la primera vez y después se actualiza desde el panel.

## 🩺 Resolución de problemas

| Ves | Porque | Haz |
|---|---|---|
| Perdiste la contraseña o el nodo ya no se une a la red | Un nodo con puesta en marcha y sin dirección tras 90 s abre su propio Wi-Fi `ARMOR-SETUP-xxxxxx` (abre `http://192.168.4.1/`) | Para empezar de cero mantén BOOT 8 segundos en los primeros 30 segundos tras encender (borra ajustes y usuarios), o usa *Restablecer de fábrica* en el panel |
| Se rechaza una actualización desde GitHub | La versión no trae suma de comprobación ni firma, no coinciden (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), no trae imagen para esta placa (`e_no_asset_for_board`) o es de otro proyecto (`e_wrong_firmware`) | El nodo no cambió; reintenta o deja que un administrador suba el fichero a mano |
| La voz no arranca | `voice.enabled` está apagado, o `voice.url` es `required` o `invalid` | Enciéndelo en el panel y da una dirección válida; cambiar de host borra el token |
| El pie dice enviando, luego confirmada o el motivo | Una orden va en camino, se confirmó unos segundos, o se rechazó y conserva su motivo | Lee el motivo; decide el servidor, no el panel |

## 📂 Estructura del repositorio

<p align="center">
  <img src="images/repository-structure.svg" alt="Estructura del repositorio de ARMOR-NODE-HMI" width="100%">
</p>

<details>
<summary>Versión en texto del árbol</summary>

```text
ARMOR-NODE-HMI/
├── main/                                el firmware: un componente de ESP-IDF
├── core/                                la lógica sin ESP-IDF, probada en un ordenador
├── panel/                               el panel web embebido en el firmware
├── tests/                               pruebas de host y comprobaciones Python
├── docs/                                9 guías: diseño, voz, fuentes, hardware
├── tools/                               herramientas de compilación, empaquetado y comprobación
├── .github/                             flujo de CI y plantillas de issues y pull requests
├── images/                              banner, icono y las dos imágenes de este README
├── build.sh build.bat                   compilar con el flujo de proyecto compartido
├── test.sh test.bat                     ejecutar las pruebas con el flujo compartido
├── run.sh run.bat                       arrancar con el flujo de proyecto compartido
├── armor.project.json                   manifiesto: versión, rol, madurez, comando de build
├── CHANGELOG.md                         qué cambió en cada versión
├── SECURITY.md SUPPORT.md LICENSE       política de seguridad, soporte y licencia
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   cómo contribuir y el código de conducta
├── CMakeLists.txt                       el proyecto ESP-IDF
├── partitions.csv                       la tabla de la flash
├── sdkconfig.defaults                   los ajustes del firmware
├── sdkconfig.board.lcd7box              los ajustes de una placa
├── secrets/                             ejemplo del fichero del nodo (los reales no se versionan)
├── dependencies.lock                    las versiones fijadas de los componentes de ESP-IDF
├── build-test.sh build-test.bat         ejecutar las pruebas con el flujo compartido
└── .gitattributes .gitignore            ajustes de Git
```

</details>

## 🗺️ Hoja de ruta y límites conocidos

**Límites conocidos hoy**

* El firmware se ha compilado una vez (ESP-IDF 5.5.5), pero la compilación no la comprueba la integración continua y nunca ha funcionado en una placa.
* El intercambio con el servidor y con el servicio de voz ARMOR-VOICE-AI se prueba contra simulaciones; es pulsar para hablar, sin palabra de activación.
* La página web se ha ejecutado contra una simulación en un navegador real; la pantalla, el táctil y el sonido están escritos a partir de los ejemplos de Waveshare y esperan su primer encendido.
* El firmware no ha funcionado en la placa: nada ha funcionado todavía con equipos reales.
* El código de configuración, el límite de accesos, el PIN Bluetooth, el manejo de la coacción y las actualizaciones firmadas están probados en el host; no ha habido una revisión independiente.
* Las comprobaciones de hash y de firma pasan en el host; una actualización por aire nunca se ha ejecutado en una placa.
* Los mensajes se validan contra los esquemas compartidos de ARMOR-COMMON; el enlace con el servidor y con el servicio de voz no se ha probado desde una placa.

**Lo siguiente**

* Hacer la primera compilación completa y el primer encendido en la placa, y después el servicio de voz en la Jetson, una palabra de activación y la vista de cámaras en la pantalla.

## 🌍 Idiomas

El README y los textos existen en inglés, español, francés, italiano, alemán, chino, japonés y portugués de Portugal.

## 🔗 Proyectos relacionados

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) es un sistema de seguridad perimetral hecho de repositorios independientes. Cada uno tiene su propia versión, sus propias pruebas y su propio README; esta es la familia:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contratos de mensajes, validadores, vectores de conformidad y tipos generados
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware de nodo de campo para ESP32-S3 con tres radares y su propio panel web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocolos de inversores y baterías solares y los mensajes de un nodo pasarela
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Nodo eléctrico: medidores, el mensaje de las lecturas de la red y las reglas de maniobra
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nodo y panel de alarma: zonas, armado, retardos, sirena y PIN, con el servidor o sin él
* **ARMOR-NODE-HMI** (este repositorio) - Panel táctil: el estado del sistema en una pantalla de pared, armar y reconocer, y el hogar del asistente de voz
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - La red local: sus dispositivos, internet y lo que cambia
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordinador central: telemetría, alarmas, dispositivos, lecturas solares y cámaras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Consola web: cámaras, radar, alarmas, energía solar y el diseñador 2D/3D del emplazamiento
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Cliente de operador Android con radar 2D/3D en vivo
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Cliente de operador para iPhone
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Política de inferencia visual que explica sus decisiones y nunca actúa
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zonas de cámara, máscaras y reglas de detección para la Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intenciones de voz sin conexión con una confirmación que no se puede falsificar
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Cajas, electrónica y la matriz de aceptación en banco
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Despliegue, el banco de pruebas de la CM5, copias de seguridad y TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulador de telemetría sin conexión con fallos repetibles
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Detecta, instala y actualiza los repositorios del propio ecosistema
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Arquitectura, base de seguridad y la matriz de capacidades

## 📚 Documentación

* [Diseño](docs/DESIGN.md) y [hardware](docs/HARDWARE.md): lo que muestra la central y la placa
* [Voz](docs/VOICE.md) y [fuentes](docs/FONTS.md)
* [Firmware del nodo](docs/NODE_FIRMWARE.md) y [acciones de compilación](docs/FIRMWARE_BUILD_ACTIONS.md)
* [Pruebas de banco](docs/BENCH_TESTS.md), [código de puesta en marcha](docs/SETUP_CODE.md) y [alta por Bluetooth](docs/BLE_PROVISIONING.md)
* [Matriz de capacidades: qué está demostrado y qué no](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catálogo de proyectos: versiones y cómo dependen unos repositorios de otros](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Registro de cambios de este repositorio](CHANGELOG.md)

## 🤝 Contribuir y comunidad

* Antes de un pull request ejecuta `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py`: debe pasar. Consulta [CONTRIBUTING.md](CONTRIBUTING.md) y el [código de conducta](CODE_OF_CONDUCT.md).
* El README existe en ocho idiomas y se genera desde las fuentes de ARMOR-DOCS: un cambio de texto se hace allí.
* Un cambio en un mensaje entre componentes empieza en ARMOR-COMMON.
* No describas como funcionando en una placa lo que solo se ha compilado.
* Informa de una vulnerabilidad en privado: [SECURITY.md](SECURITY.md). Preguntas, ideas e informes: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTOR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENCIA

GPL-3.0-or-later - véase [LICENSE](LICENSE).
