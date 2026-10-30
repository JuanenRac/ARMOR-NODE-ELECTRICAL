<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="Banner de ARMOR-NODE-ELECTRICAL" width="100%">
</p>

# ⚡ ARMOR-NODE-ELECTRICAL

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

### Nodo eléctrico: lee las tensiones, corrientes, potencia y energía de la red AC y DC de la casa y guarda las reglas para maniobrarla (núcleo probado en el PC y un firmware que compila; nunca ha corrido en una placa)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 o posterior">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Lenguaje">
  <img src="https://img.shields.io/badge/Meters-PZEM--004T%20%2F%20017-ffb020.svg" alt="Meters">
  <img src="https://img.shields.io/badge/Checks-376%2C195-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-382264%20checks-2ea44f.svg" alt="Pruebas">
  <img src="https://img.shields.io/badge/Version-0.5.1-0f6cbd.svg" alt="Versión">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Madurez">
</p>

---

**Comprobación de honestidad - qué funciona hoy:** El núcleo (las tramas de los medidores PZEM-004T v3 y PZEM-017, el bus que los pregunta, el mensaje del nodo, las entradas analógicas y las reglas de conmutación: 382.264 comprobaciones en 19 programas de test, 375.036 de ellas sobre las reglas de conmutación) se prueba en un ordenador con medidores y contactores de pega, y los mensajes que hace los acepta ARMOR-COMMON. El firmware del nodo (los ajustes, la línea serie, las páginas Medidores y Lecturas de su panel, Bluetooth, actualizaciones firmadas) compila en el contenedor ESP-IDF 5.5.5 para las dos placas. **Nunca ha corrido en una placa, no se ha conectado nada a un medidor ni a un contactor, y nada de aquí conmuta nada.**

---

## 📋 De un vistazo

| | |
|---|---|
| Versión | `0.5.1` |
| Madurez | Funcional |
| Función | Nodo de campo eléctrico: medidores, entradas y maniobra (familia: Field Nodes, padre: ARMOR-COMMON) |
| Tecnología | C++17, ESP-IDF 5.5.5, ESP32-S3 |
| Pruebas | 382264 comprobaciones en 19 programas de host y 5 de Python (`ctest`) |
| Equipos | Medidores PZEM-004T v3 y PZEM-017 (hasta 16), entradas analógicas, relés; maniobra apagada por defecto |
| Funciona en | Una placa ESP32-S3 (Ethernet o Wi-Fi); pruebas de host en cualquier ordenador |
| En hardware real | Ninguno todavía: ninguna placa montada ni medidor o contactor conectado; nada conmuta nada |
| Licencia | GPL-3.0-or-later |

**Ir a:** [Resumen](#-resumen) · [Arquitectura](#-arquitectura) · [Seguridad](#-seguridad-y-protección) · [Interfaces](#-interfaces) · [Configuración](#-configuración) · [Inicio rápido](#-inicio-rápido) · [Pruebas](#-pruebas) · [Problemas](#-resolución-de-problemas) · [Estructura](#-estructura-del-repositorio) · [Relacionados](#-proyectos-relacionados)

## 🚦 Estado del proyecto

Cada línea es una parte de este repositorio y la luz dice hasta dónde llega hoy. Está escrito con lo que el repositorio puede demostrar (pruebas, compilaciones, ejecuciones), no con lo que está planeado.

🟢 Hecho y verificado · 🟡 Funciona, con límites conocidos · 🟠 Parcial o sin verificar todavía · 🔴 No hecho o no funciona

| Área | Estado | Qué significa aquí |
|---|---|---|
| Software | 🟢 | El firmware compila en el contenedor ESP-IDF 5.5.5 para las dos placas (Wi-Fi y Ethernet). |
| Pruebas | 🟢 | Pasan 382.264 comprobaciones en 19 programas de host (`ctest`), 375.036 de ellas sobre las reglas de maniobra; los medidores y contactores son simulaciones. |
| Protocolos y datos | 🟡 | Las tramas y lecturas de los PZEM-004T v3 y PZEM-017 se comprueban contra las tramas documentadas y muestras grabadas; ningún medidor ha respondido todavía. |
| Interfaz | 🟡 | Las páginas de medidores y lecturas del panel web se han ejercitado en un navegador contra un nodo simulado, no en una placa. |
| Hardware | 🔴 | Ninguna placa construida ni medidor o contactor conectados: nada ha funcionado todavía con equipos reales. |
| Seguridad | 🟡 | El código de configuración, el acceso con límite de contraseñas erróneas, el PIN Bluetooth y las actualizaciones firmadas están probados en el host; no ha habido una revisión independiente. |
| Actualizaciones | 🟠 | Las comprobaciones de hash y de firma Ed25519 y la lógica de vuelta atrás pasan en el host; una actualización por aire nunca se ha ejecutado en una placa. |
| Escritura en los equipos | 🟠 | Las reglas para maniobrar contactores solo se han probado contra contactores simulados; nada de aquí ha maniobrado nada real. |
| Integración | 🟠 | Los mensajes del nodo se validan contra los esquemas compartidos y los vectores de conformidad de ARMOR-COMMON. Su cliente MQTT se prueba contra un Mosquitto real en los tests; ninguna placa los ha enviado aún a un servidor. |
| Documentación | 🟢 | Ocho idiomas, la guía del firmware, las notas de cableado y el historial de cambios están al día. |

## 🎯 Resumen

* **Lectura de contadores:** las tramas Modbus RTU de los **PZEM-004T v3** (AC) y **PZEM-017** (DC) de Peacefair: el CRC, la petición que lee los registros y la decodificación de la respuesta (tensión, corriente, potencia, energía, frecuencia, factor de potencia, alarma). Solo se construye la petición de lectura: las que escriben en un contador (su dirección, sus umbrales, el reinicio de su energía) no se construyen en ningún sitio. Se rechaza una respuesta que no cuadra o que trae un valor que ningún contador real podría dar.
* **El bus:** una línea con varios contadores se pregunta de uno en uno, con un tiempo de espera; se guarda la última lectura buena de cada uno y un contador que calla deja de publicarse a los diez segundos.
* **El mensaje** `armor/electrical/<nodo>/state`: una entrada por canal (un circuito, una línea, la entrada de red, un bus DC) con AC o DC, tensión, corriente, potencia, energía, frecuencia, factor de potencia, el estado de un interruptor tal como lo ve el nodo y una alarma; está en el contrato compartido y lleva un estado, nunca una orden.
* **Las reglas para maniobrar,** aparte de cualquier hardware: un controlador del cambio entre dos fuentes que nunca manda las dos, exige la maniobra permitida, el nodo armado justo antes y los dos contactores confirmados abiertos durante todo el tiempo muerto, y convierte un contactor que no muestra lo que se le ordenó en una avería que se queda hasta que se reconoce. **La maniobra está desactivada por defecto y nada maneja ningún hardware.**
* **Las órdenes a un conmutador, sin activar:** el mensaje de estado puede llevar el estado de los conmutadores del nodo, y el contrato compartido tiene una orden (`arm`, y después `close_a` o `close_b` con un token de un solo uso, `open`, `acknowledge`) y la respuesta del nodo; el núcleo las lee y las contesta con las reglas anteriores, probado con los vectores compartidos y con contactores simulados. Ninguna imagen del firmware lo incluye, así que nada puede accionar un contactor, y ARMOR-SERVER se niega a enviar una orden si no se activó (`ARMOR_ELECTRICAL_SWITCHING=1`) y el ACL del broker no lo permite.
* **Dónde se ve:** el *Diseñador eléctrico* de ARMOR-STUDIO dibuja la red de la casa y muestra en cada elemento lo que mide su canal; ARMOR-SERVER guarda las lecturas, su historial y las sumas.
* **El firmware del nodo** (ESP32-S3-WROOM-1 N16R8 por Wi-Fi, o la Waveshare ESP32-S3-ETH por su cable): los ajustes, una línea serie para hasta dieciséis contadores y el panel web de los otros nodos (puesta en marcha, usuarios, Wi-Fi, broker, actualización por aire, HTTPS) con sus propias páginas de Contadores y Lecturas, en ocho idiomas. Solo lee: las reglas para maniobrar no están enlazadas en él. Véase [el firmware](docs/NODE_FIRMWARE.md).
* **Configuración desde el móvil por Bluetooth,** el mismo canal que el del nodo radar: la app ARMOR encuentra el nodo como `ARMOR-XXXXXX` y ajusta su nombre, Wi-Fi, dirección, broker y modo de Bluetooth con los usuarios y el código de puesta en marcha del panel ([el protocolo](docs/BLE_PROVISIONING.md)). Solo escucha mientras el nodo no tiene usuarios, salvo que se indique otra cosa. La parte de radio nunca ha corrido en una placa.
* **Actualizaciones por aire firmadas:** el nodo solo instala desde GitHub una imagen cuyo SHA-256 coincide y que lleva una firma Ed25519 válida del responsable (la clave pública está en el firmware, la privada nunca sale del responsable); un archivo subido a mano por un administrador en el panel se sigue aceptando. Las páginas y mensajes del panel están en ocho idiomas.
* **Emparejamiento Bluetooth con PIN:** el PIN de seis cifras se deriva del código de configuración, se imprime en la consola USB y se muestra a un administrador en el panel; el móvil tiene que escribirlo antes de poder cambiar los ajustes del nodo.
* **Todavía no:** el firmware funcionando en una placa, el hardware, ninguna medida de una instalación real y ninguna maniobra.
* **Solo la casa puede usar el panel:** el panel responde por defecto a la red privada (o a las redes que su dueño indique, como una VPN), rechaza una lista que dejaría fuera a quien la guarda, cuenta las contraseñas erróneas por persona y por dirección, y funciona detrás del proxy del servidor para llegar a él desde fuera de casa sin publicar el nodo.
* **Las estimaciones lo dicen:** un canal hecho con una entrada de corriente lleva `power_estimated`, así que Studio y el servidor muestran su potencia como estimada (potencia aparente a partir de una corriente y una tensión nominal) y marcan una cifra de red que incluya una.
* **Enlace cifrado opcional con el broker (TLS):** desactivado por defecto, así que el nodo habla MQTT sin cifrar como antes. Un interruptor y una caja para el certificado de la autoridad propia de la instalación (PEM, 2 KB como máximo) en la página Broker hacen que se conecte con `mqtts://`, confíe solo en esa autoridad y compruebe que el certificado del broker se hizo para la dirección a la que llegó; lo que no sea un único certificado PEM se rechaza. Cubierto con pruebas en el ordenador; el handshake aún no se ha ejecutado en una placa.
* **Informe de salud:** cada minuto más o menos el nodo publica cuánto lleva encendido y por qué se reinició la última vez, su memoria libre, su enlace y su señal Wi-Fi, cuántas veces se conectó al broker y el último error que anotó (`armor/health/<nodo>`, el contrato `node_health` de ARMOR-COMMON). El código que lo construye lo comparten todos los nodos y está probado en un ordenador; no se ha ejecutado en una placa.
* **Control desde el servidor:** Studio puede reiniciar el nodo limpiamente, pausarlo hasta una hora (se reanuda solo), poner su trabajo en espera (sin fin, se conserva al reiniciar, el nodo sigue conectado y respondiendo), hacer que se muestre, pedirle una instantánea de diagnóstico y marcarlo en mantenimiento (`armor/control/<nodo>/...`, el contrato `node_control` de ARMOR-COMMON). Cada orden son dos pasos con un token de un solo uso que pertenece a esa operación, solo el servidor puede escribirlas en el broker, y no existe restablecimiento de fábrica, borrado ni sueño profundo. Mientras el nodo está en pausa o en espera todos los relés se abren y ninguno se mueve, lo pida quien lo pida (el panel, el broker o una central); los contadores se siguen leyendo. El código es compartido por todos los nodos y se prueba en un ordenador; no ha funcionado en una placa.
* **Lecturas que dicen lo frescas que son:** cada canal de medidor lleva la edad de su lectura y cuántas peticiones al medidor han fallado, así que el servidor distingue un 0 W real de un medidor que dejó de responder; y una tarea de medidores colgada reinicia la placa mediante el watchdog.
* **Diagnóstico sin cable:** tras un arranque que no fue limpio (un cuelgue, un watchdog, una caída de la alimentación, una reserva de memoria fallida) el informe de salud dice el motivo durante diez minutos; un fallo deja su volcado de núcleo (core dump) para verlo, descargarlo o borrarlo desde el panel; los ajustes guardados se copian antes de cada instalación de firmware y se recuperan si una actualización los daña; y un administrador puede pedir al nodo, a través del servidor, un paquete de diagnóstico o su registro con un token de un solo uso.
* **Enclavamiento de relés:** un relé puede nombrar otro que nunca debe estar cerrado a la vez (las dos fuentes de una carga, los dos sentidos de un motor); se rechaza cerrar uno mientras el otro está cerrado, desde el panel, el broker y la central por igual, y el nodo nunca abre el otro por su cuenta. Probado en un ordenador; no se ha ejecutado en una placa.
* **Lectura, solo lectura, de los ajustes propios de los contadores:** tras la primera lectura buena y cada `bus.check_settings_s` (600 s, 0 = nunca) el nodo lee los registros holding del PZEM (004T v3: umbral de potencia y dirección; 017: umbrales de tensión, dirección y shunt) en el mismo turno que las medidas, y avisa si la dirección guardada no es la que usa; no se escribe nada en el contador.
* **Sellado opcional de los secretos guardados:** desactivado por defecto (`security.seal_secrets`, o `nodes.nvs_encryption` del servidor). Activado, las contraseñas, tokens, claves, PIN y hashes de contraseña que guarda el nodo quedan sellados con ChaCha20-Poly1305 y todo lo demás sigue legible; activarlo o desactivarlo los reescribe sin perder nada, aunque el nodo se pare a medias. No se quema ningún eFuse ni se cifra la flash: la clave está en una partición del mismo chip, así que **no** protege frente a quien tenga la placa, y un nodo instalado antes de esa partición conserva sus secretos como estaban hasta que se grabe con la nueva tabla de particiones.

## 🧩 Papel en A.R.M.O.R.

ARMOR-NODE-ELECTRICAL mide la red de CA y CC de la casa con sus medidores y guarda las reglas para conmutar entre fuentes. Publica las lecturas para [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), que guarda su historial y la energía de cada día y genera las alarmas; el menú *Eléctrico* y el *Diseñador eléctrico* de [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) y la pantalla eléctrica de [ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL) lo muestran. La conmutación está diseñada (armar, un token de un solo uso, el enclavamiento, la confirmación de los dos contactores) pero no activada.

| Nombre | Cómo | Para qué |
|---|---|---|
| ARMOR-SERVER | MQTT `armor/electrical/<id>/{state,command,result}` | Recibe los canales y ordena una transferencia (solo si la maniobra está activada en tres sitios) |
| ARMOR-STUDIO | menú Eléctrico, Electrical Designer | Muestra los canales y las alarmas y abre el panel del nodo |
| ARMOR-ANDROID-CONTROL | Bluetooth | Configura el nodo en el campo y muestra los canales |
| ARMOR-COMMON | contrato eléctrico | Los mensajes se comprueban con sus esquemas; el firmware común se copia de él |

## 🏛️ Arquitectura

<p align="center">
  <img src="images/architecture.svg" alt="Arquitectura de ARMOR-NODE-ELECTRICAL: quién lo llama, qué contiene y de qué depende" width="100%">
</p>

## 🖼️ Capturas de pantalla

Capturadas a 1920×1080 en un banco de pruebas, con datos de ejemplo y sin ninguna instalación real detrás.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Resumen: el estado del nodo de un vistazo - red, broker, firmware y lo que reporta." width="100%"><br><sub>Resumen: el estado del nodo de un vistazo - red, broker, firmware y lo que reporta.</sub></td><td width="50%" valign="top"><img src="images/screenshots/meters.png" alt="Contadores: los contadores de energía y canales analógicos que lee el nodo, cada uno con su nombre y escala." width="100%"><br><sub>Contadores: los contadores de energía y canales analógicos que lee el nodo, cada uno con su nombre y escala.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/readings.png" alt="Lecturas: lo que el nodo lee ahora mismo, con la antigüedad de cada valor." width="100%"><br><sub>Lecturas: lo que el nodo lee ahora mismo, con la antigüedad de cada valor.</sub></td><td width="50%" valign="top"><img src="images/screenshots/zigbee.png" alt="Radio Zigbee: el puente serie hacia el módulo coordinador, qué cliente puede usarlo y el estado del puente." width="100%"><br><sub>Radio Zigbee: el puente serie hacia el módulo coordinador, qué cliente puede usarlo y el estado del puente.</sub></td></tr>
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
| Una orden remota que hace daño | Dos pasos con un token de un solo uso ligado a esa operación; solo el servidor puede escribir esos topics en el broker; la pausa dura una hora como máximo; no hay restablecimiento de fábrica, borrado ni sueño profundo por orden remota |
| Secretos guardados leídos por quien tiene la placa | Opcional (apagado por defecto, interruptor `security.seal_secrets` o `nodes.nvs_encryption` del servidor): ChaCha20-Poly1305 para contraseñas, tokens, claves, PIN y hashes. No cifra la flash ni quema un eFuse: la clave está en una partición del mismo chip, así que no protege frente a quien tenga la placa |
| El enlace con el broker en claro | TLS opcional (`mqtts://`) con la autoridad propia del dueño (PEM de hasta 2 KB) y comprobación del nombre; apagado por defecto. El handshake solo se ha ejecutado en pruebas de host, no en una placa |
| Red o continua peligrosa en la placa | El nodo solo lee los módulos PZEM (módulos cerrados, línea serie aislada) y nunca lleva red en su placa; lo que conmuta es una etapa aparte probada en banco con lámpara y transformador de aislamiento |
| Dos fuentes cerradas a la vez | El controlador de transferencia nunca ordena las dos: exige maniobra permitida, nodo armado justo antes y ambos contactores confirmados abiertos durante todo el tiempo muerto (`dead_time_ms` 2000 por defecto); un contactor que no muestra lo ordenado es un fallo que se queda hasta reconocerlo (`kDidNotOpen`, `kDidNotClose`); sigue siendo obligatorio un enclavamiento mecánico entre los contactores |
| Una orden repetida, o una maniobra que nadie pidió | `arm` da un token de un solo uso de 64 bits que `close_a` o `close_b` deben llevar, dentro de `arm_window_ms`; las órdenes nunca se retienen; máximo 4 interruptores; la maniobra está APAGADA salvo que el firmware la permita (`switching_enabled`), el servidor corra con `ARMOR_ELECTRICAL_SWITCHING=1` y la lista de acceso del broker esté abierta para ese nodo |
| Un fallo, una realimentación perdida o la maniobra apagada | Cada controlador responde con TODAS las bobinas abiertas (fallo => abierto; si abierto es seguro para la carga lo decide la instalación); el estado del mensaje es lo que muestra el contacto auxiliar, no lo ordenado; las parejas de relés están enclavadas y el nodo nunca abre la pareja por sí mismo |
| Una lectura vieja mostrada como real | Un medidor mudo deja de publicarse a los 10 s; cada canal lleva la antigüedad de su lectura y el número de peticiones fallidas; una tarea de medidores colgada reinicia la placa por watchdog; cada `bus.check_settings_s` (600 s, 0 = nunca) se leen los ajustes de los medidores (solo lectura) y se avisa si cambió una dirección |

La red (230 V CA) y la corriente continua de un banco de baterías pueden matar y provocar incendios. [docs/SAFETY.md](docs/SAFETY.md) dice lo que supone el diseño; no es el diseño de una instalación y no sustituye la normativa del país ni a un instalador cualificado. Hoy nada de esto conmuta nada.

## 🔌 Interfaces

| Parte | Qué define |
|---|---|
| MQTT | `armor/electrical/<id>/state` (un canal por entrada: CA o CC, tensión, corriente, potencia, energía, frecuencia, factor de potencia, estado del interruptor, alarma; contrato de ARMOR-COMMON), `armor/electrical/<id>/command` y `/result`, `armor/device/<id>/<relé>/{state,set}`, `armor/health/<id>`, `armor/control/<id>/...` |
| Órdenes | `arm`, `close_a`, `close_b` (con el token), `open` y `acknowledge` (borra un fallo) |
| Medidores | Peacefair PZEM-004T v3 (CA) y PZEM-017 (CC) por Modbus RTU, hasta 16 en una línea serie; solo se construye la petición de lectura (la que escribe dirección o umbrales no existe); entradas analógicas, contactos y relés en la placa base; un puente Zigbee ([docs/ZIGBEE.md](docs/ZIGBEE.md)) |

## ⚙️ Configuración

| Variable | Por defecto | Qué define |
|---|---|---|
| `bus.check_settings_s` | 600 | Segundos entre las comprobaciones de solo lectura de los ajustes de los medidores (0 = nunca) |
| `dead_time_ms`, `confirm_timeout_ms` | 2000 | El tiempo muerto de una transferencia y cuánto tiene un contactor para mostrar lo ordenado |
| `switching_enabled` | off | La maniobra está APAGADA salvo que este ajuste, el servidor (`ARMOR_ELECTRICAL_SWITCHING=1`) y la lista de acceso del broker la permitan |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | Los ajustes que comparten todos los nodos: el nombre (vacío da `armor-` y los últimos bytes de la MAC), el código de puesta en marcha (vacío: aleatorio), el secreto de flota (16+ caracteres, nunca en el repositorio), los primeros ajustes del broker, el latido (de 2 a 300 s), los minutos de puesta en marcha por Bluetooth (de 1 a 120) y el reinicio con BOOT (8 s en los primeros 30 s) |

## 🚀 Inicio rápido

**Requisitos:** un compilador C++17 para las pruebas de host; el contenedor ESP-IDF 5.5.5 para compilar la imagen; una placa ESP32-S3 (Ethernet o Wi-Fi).

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel without a board
tools/build_node.sh generic s3-wifi                  # the image in the ESP-IDF container (or s3-eth)
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

Véanse el [diseño](docs/DESIGN.md), las [notas de seguridad](docs/SAFETY.md), las [reglas para maniobrar](docs/SWITCHING.md), los [protocolos](docs/PROTOCOLS.md) y los [mensajes](docs/ELECTRICAL_MESSAGES.md).

## 🧪 Pruebas

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 programas de prueba de host y 5 comprobaciones en Python se ejecutan con `ctest`; 382264 comprobaciones contadas por los programas de host, compilados con `-Werror`.

Comprobaciones Python: los textos del panel en ocho idiomas, las tareas, las rutas de guardado, la sesión Bluetooth bajo su bloqueo y las rutas que llama el panel (y, en algunos nodos, los códigos HTTP o los glifos de las fuentes)

Las pruebas de host necesitan cualquier compilador C++17 (Linux, WSL, MSYS2); el panel web también se ejercita en un navegador real contra un nodo de pruebas (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

La mayoría de las comprobaciones son barridos exhaustivos de las reglas de maniobra (`test_switch_set` y `test_interlock`: 375 036); los medidores y contactores son sustitutos. `test_clock` solo imprime `ok`.

## 🏗️ Compilación y publicación

`tools/build_node.sh` compila la imagen en el contenedor ESP-IDF 5.5.5 y la versión lleva la imagen, su SHA-256 y su firma Ed25519, una por placa; la imagen se graba por USB-C la primera vez y después se actualiza desde el panel.

## 🩺 Resolución de problemas

| Ves | Porque | Haz |
|---|---|---|
| Perdiste la contraseña o el nodo ya no se une a la red | Un nodo con puesta en marcha y sin dirección tras 90 s abre su propio Wi-Fi `ARMOR-SETUP-xxxxxx` (abre `http://192.168.4.1/`) | Para empezar de cero mantén BOOT 8 segundos en los primeros 30 segundos tras encender (borra ajustes y usuarios), o usa *Restablecer de fábrica* en el panel |
| Se rechaza una actualización desde GitHub | La versión no trae suma de comprobación ni firma, no coinciden (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), no trae imagen para esta placa (`e_no_asset_for_board`) o es de otro proyecto (`e_wrong_firmware`) | El nodo no cambió; reintenta o deja que un administrador suba el fichero a mano |
| Un conmutador muestra un fallo | El contactor no mostró abierto o cerrado como se ordenó (`kDidNotOpen`, `kDidNotClose`); se queda hasta reconocerlo | Revisa el cableado del contacto auxiliar y envía `acknowledge` |
| Se rechaza una orden | La maniobra está apagada (`switching_enabled` falso), el token no es válido o caducó, o el nodo no estaba armado justo antes | Arma otra vez y manda la orden con el token nuevo |

## 📂 Estructura del repositorio

<p align="center">
  <img src="images/repository-structure.svg" alt="Estructura del repositorio de ARMOR-NODE-ELECTRICAL" width="100%">
</p>

<details>
<summary>Versión en texto del árbol</summary>

```text
ARMOR-NODE-ELECTRICAL/
├── main/                                el firmware: un componente de ESP-IDF
├── core/                                la lógica sin ESP-IDF, probada en un ordenador
├── panel/                               el panel web embebido en el firmware
├── tests/                               pruebas de host y comprobaciones Python
├── docs/                                12 guías: seguridad, maniobra, mensajes, hardware
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
├── sdkconfig.board.s3-eth               los ajustes de una placa
├── sdkconfig.board.s3-wifi              los ajustes de una placa
├── secrets/                             ejemplo del fichero del nodo (los reales no se versionan)
├── build-test.sh build-test.bat         ejecutar las pruebas con el flujo compartido
└── .gitattributes .gitignore            ajustes de Git
```

</details>

## 🗺️ Hoja de ruta y límites conocidos

**Límites conocidos hoy**

* Las tramas y lecturas de los PZEM-004T v3 y PZEM-017 se comprueban contra las tramas documentadas y muestras grabadas; ningún medidor ha respondido todavía.
* Las páginas de medidores y lecturas del panel web se han ejercitado en un navegador contra un nodo simulado, no en una placa.
* Ninguna placa construida ni medidor o contactor conectados: nada ha funcionado todavía con equipos reales.
* El código de configuración, el acceso con límite de contraseñas erróneas, el PIN Bluetooth y las actualizaciones firmadas están probados en el host; no ha habido una revisión independiente.
* Las comprobaciones de hash y de firma Ed25519 y la lógica de vuelta atrás pasan en el host; una actualización por aire nunca se ha ejecutado en una placa.
* Las reglas para maniobrar contactores solo se han probado contra contactores simulados; nada de aquí ha maniobrado nada real.

**Lo siguiente**

* Probar los medidores, los contactores y las reglas de maniobra en un banco con lámpara y transformador de aislamiento, y después una placa real con la actualización por aire.

## 🌍 Idiomas

El README y los textos existen en inglés, español, francés, italiano, alemán, chino, japonés y portugués de Portugal.

## 🔗 Proyectos relacionados

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) es un sistema de seguridad perimetral hecho de repositorios independientes. Cada uno tiene su propia versión, sus propias pruebas y su propio README; esta es la familia:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contratos de mensajes, validadores, vectores de conformidad y tipos generados
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware de nodo de campo para ESP32-S3 con tres radares y su propio panel web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocolos de inversores y baterías solares y los mensajes de un nodo pasarela
* **ARMOR-NODE-ELECTRICAL** (este repositorio) - Nodo eléctrico: medidores, el mensaje de las lecturas de la red y las reglas de maniobra
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nodo y panel de alarma: zonas, armado, retardos, sirena y PIN, con el servidor o sin él
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Panel táctil: el estado del sistema en una pantalla de pared, armar y reconocer, y el hogar del asistente de voz
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

* [Seguridad](docs/SAFETY.md) y [maniobra](docs/SWITCHING.md): qué supone el diseño y cómo va una transferencia
* [Mensajes](docs/ELECTRICAL_MESSAGES.md), [protocolos](docs/PROTOCOLS.md) y [Zigbee](docs/ZIGBEE.md)
* [Diseño](docs/DESIGN.md), [hardware](docs/HARDWARE.md), [placa base](docs/BASE_BOARD.md) y [firmware del nodo](docs/NODE_FIRMWARE.md)
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
