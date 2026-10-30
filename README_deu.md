<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-ELECTRICAL-Banner" width="100%">
</p>

# ⚡ ARMOR-NODE-ELECTRICAL

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  🇩🇪 <b>Deutsch</b> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### Elektroknoten: liest Spannungen, Ströme, Leistung und Energie des AC- und DC-Netzes des Hauses und hält die Regeln fürs Schalten (am Rechner getesteter Kern und eine Firmware, die baut; sie lief noch nie auf einer Platine)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 oder neuer">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Sprache">
  <img src="https://img.shields.io/badge/Meters-PZEM--004T%20%2F%20017-ffb020.svg" alt="Meters">
  <img src="https://img.shields.io/badge/Checks-376%2C195-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-382264%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.5.1-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Reifegrad">
</p>

---

**Ehrlichkeitsprüfung - was heute läuft:** Der Kern (die Rahmen der Zähler PZEM-004T v3 und PZEM-017, der Bus, der sie abfragt, die Nachricht des Knotens, die analogen Eingänge und die Schaltregeln: 382.264 Prüfungen in 19 Testprogrammen, davon 375.036 zu den Schaltregeln) wird auf einem Computer mit Ersatzzählern und -schützen getestet, und die von ihm erzeugten Nachrichten akzeptiert ARMOR-COMMON. Die Firmware des Knotens (die Einstellungen, die serielle Leitung, die Seiten Zähler und Messwerte seines Panels, Bluetooth, signierte Updates) baut im ESP-IDF-5.5.5-Container für beide Platinen. **Sie lief nie auf einer Platine, an keinen Zähler und kein Schütz wurde etwas angeschlossen, und nichts hier schaltet irgendetwas.**

---

## 📋 Auf einen Blick

| | |
|---|---|
| Version | `0.5.1` |
| Reifegrad | Funktionsfähig |
| Rolle | Elektro-Feldknoten: Zähler, Eingänge und Schalten (Familie: Field Nodes, übergeordnet: ARMOR-COMMON) |
| Technik | C++17, ESP-IDF 5.5.5, ESP32-S3 |
| Tests | 382264 Prüfungen in 19 Host-Programmen und 5 Python-Prüfungen (`ctest`) |
| Geräte | Zähler PZEM-004T v3 und PZEM-017 (bis zu 16), analoge Eingänge, Relais; Schalten standardmäßig aus |
| Läuft auf | Eine ESP32-S3-Platine (Ethernet oder WLAN); Host-Tests auf jedem Computer |
| Auf echter Hardware | Noch keiner: keine Platine aufgebaut und kein Zähler oder Schütz angeschlossen; nichts schaltet irgendetwas |
| Lizenz | GPL-3.0-or-later |

**Springe zu:** [Überblick](#-überblick) · [Architektur](#-architektur) · [Sicherheit](#-sicherheit-und-schutz) · [Schnittstellen](#-schnittstellen) · [Konfiguration](#-konfiguration) · [Schnellstart](#-schnellstart) · [Tests](#-tests) · [Probleme](#-fehlerbehebung) · [Struktur](#-struktur-des-repositorys) · [Verwandte](#-verwandte-projekte)

## 🚦 Projektstatus

Jede Zeile ist ein Teil dieses Repositorys, und die Ampel sagt, wie weit er heute ist. Sie beruht auf dem, was das Repository zeigen kann (Tests, Builds, Läufe), nicht auf dem, was geplant ist.

🟢 Fertig und überprüft · 🟡 Funktioniert, mit bekannten Grenzen · 🟠 Teilweise fertig oder noch nicht überprüft · 🔴 Nicht gemacht oder funktioniert nicht

| Bereich | Stand | Was das hier bedeutet |
|---|---|---|
| Software | 🟢 | Die Firmware wird im ESP-IDF-5.5.5-Container für beide Platinen (WLAN und Ethernet) gebaut. |
| Tests | 🟢 | 382.264 Prüfungen in 19 Host-Programmen bestehen (`ctest`), davon 375.036 zu den Schaltregeln; die Zähler und Schütze sind Nachbildungen. |
| Protokolle und Daten | 🟡 | Die Telegramme und Messwerte der PZEM-004T v3 und PZEM-017 werden gegen die dokumentierten Telegramme und aufgezeichnete Beispiele geprüft; noch hat kein Zähler geantwortet. |
| Oberfläche | 🟡 | Die Seiten für Zähler und Messwerte des Web-Panels wurden in einem Browser gegen einen nachgebildeten Knoten erprobt, nicht auf einer Platine. |
| Hardware | 🔴 | Keine Platine gebaut und kein Zähler oder Schütz angeschlossen: Bisher lief nichts auf echten Geräten. |
| Sicherheit | 🟡 | Der Einrichtungscode, die Anmeldung mit Begrenzung falscher Passwörter, die Bluetooth-PIN und die signierten Aktualisierungen sind auf dem Host getestet; es gab keine unabhängige Prüfung. |
| Aktualisierungen | 🟠 | Die Hash- und Ed25519-Signaturprüfungen und die Rückfalllogik bestehen auf dem Host; eine Aktualisierung über die Luft wurde nie auf einer Platine ausgeführt. |
| Schreiben auf Geräte | 🟠 | Die Schaltregeln für Schütze sind nur gegen nachgebildete Schütze getestet; nichts hier hat bisher etwas Echtes geschaltet. |
| Integration | 🟠 | Die Nachrichten des Knotens werden gegen die gemeinsamen Schemas und Konformitätsvektoren von ARMOR-COMMON geprüft. Sein MQTT-Client wird in den Tests gegen einen echten Mosquitto gespielt; noch keine Platine hat sie an einen Server geschickt. |
| Dokumentation | 🟢 | Acht Sprachen, die Firmware-Anleitung, die Verdrahtungshinweise und das Änderungsprotokoll sind aktuell. |

## 🎯 Überblick

* **Zähler lesen:** die Modbus-RTU-Rahmen der Peacefair-Zähler **PZEM-004T v3** (AC) und **PZEM-017** (DC): die CRC, die Anfrage, die die Register liest, und die Auswertung der Antwort (Spannung, Strom, Leistung, Energie, Frequenz, Leistungsfaktor, Alarm). Nur die Leseanfrage wird gebaut: die schreibenden (Adresse, Schwellen, Zurücksetzen der Energie) werden nirgends gebaut. Eine Antwort, die nicht passt oder einen Wert enthält, den kein echter Zähler liefern könnte, wird abgelehnt.
* **Der Bus:** eine Leitung mit mehreren Zählern wird einzeln abgefragt, mit Zeitlimit; der letzte gute Wert jedes Zählers wird behalten, und ein Zähler, der schweigt, wird nach zehn Sekunden nicht mehr veröffentlicht.
* **Die Nachricht** `armor/electrical/<Knoten>/state`: ein Eintrag je Kanal (ein Stromkreis, eine Leitung, der Netzanschluss, ein DC-Bus) mit AC oder DC, Spannung, Strom, Leistung, Energie, Frequenz, Leistungsfaktor, dem Zustand eines Schalters, wie ihn der Knoten sieht, und einem Alarm; sie steht im gemeinsamen Vertrag und trägt einen Zustand, nie einen Befehl.
* **Die Regeln fürs Schalten,** unabhängig von jeder Hardware: ein Regler für den Wechsel zwischen zwei Quellen, der nie beide ansteuert, das erlaubte Schalten, den kurz zuvor scharfgeschalteten Knoten und beide Schütze über die ganze Totzeit als offen bestätigt verlangt und ein Schütz, das nicht zeigt, was ihm befohlen wurde, zu einer Störung macht, die bis zur Quittierung bleibt. **Schalten ist standardmäßig aus, und nichts steuert irgendeine Hardware.**
* **Die Befehle an einen Schalter, nicht aktiviert:** Die Zustandsnachricht kann den Zustand der Schalter des Knotens tragen, und der gemeinsame Vertrag kennt einen Befehl (`arm`, dann `close_a` oder `close_b` mit einem Einmal-Token, `open`, `acknowledge`) und die Antwort des Knotens; der Kern liest und beantwortet sie mit den obigen Regeln, geprüft mit den gemeinsamen Vektoren und mit simulierten Schützen. Kein Firmware-Abbild enthält es, also kann nichts ein Schütz ansteuern, und ARMOR-SERVER sendet keinen Befehl, solange er nicht eingeschaltet wurde (`ARMOR_ELECTRICAL_SWITCHING=1`) und die ACL des Brokers ihn nicht erlaubt.
* **Wo man es sieht:** der *Elektroplaner* von ARMOR-STUDIO zeichnet das Netz des Hauses und zeigt an jedem Element, was sein Kanal misst; ARMOR-SERVER speichert die Messwerte, ihren Verlauf und die Summen.
* **Die Firmware des Knotens** (ESP32-S3-WROOM-1 N16R8 über WLAN oder das Waveshare ESP32-S3-ETH am Kabel): die Einstellungen, eine serielle Leitung für bis zu sechzehn Zähler und das Web-Panel der anderen Knoten (Einrichtung, Benutzer, WLAN, Broker, Update über die Luft, HTTPS) mit eigenen Seiten für Zähler und Messwerte, in acht Sprachen. Sie liest nur: die Regeln fürs Schalten sind nicht eingebunden. Siehe [die Firmware](docs/NODE_FIRMWARE.md).
* **Konfiguration vom Handy über Bluetooth,** derselbe Kanal wie beim Radarknoten: Die ARMOR-App findet den Knoten als `ARMOR-XXXXXX` und stellt Name, WLAN, Adresse, Broker und Bluetooth-Modus mit den Benutzern und dem Einrichtungscode des Panels ein ([das Protokoll](docs/BLE_PROVISIONING.md)). Er lauscht nur, solange der Knoten keinen Benutzer hat, sofern nichts anderes eingestellt ist. Der Funkteil lief noch nie auf einer Platine.
* **Signierte Over-the-Air-Updates:** der Knoten installiert von GitHub nur ein Abbild, dessen SHA-256 stimmt und das eine gültige Ed25519-Signatur des Betreuers trägt (der öffentliche Schlüssel steckt in der Firmware, der private verlässt den Betreuer nie); eine von einem Administrator im Panel von Hand hochgeladene Datei wird weiterhin akzeptiert. Seiten und Meldungen des Panels sind in acht Sprachen.
* **Bluetooth-Kopplung mit PIN:** die sechsstellige PIN wird aus dem Einrichtungscode abgeleitet, auf der USB-Konsole ausgegeben und einem Administrator im Panel angezeigt; das Telefon muss sie eingeben, bevor es die Einstellungen des Knotens schreiben darf.
* **Noch nicht:** die Firmware auf einer Platine, die Hardware, jede Messung einer echten Anlage und jedes Schalten.
* **Nur das Haus darf das Panel nutzen:** Das Panel antwortet standardmäßig dem privaten Netz (oder den Netzen, die der Besitzer nennt, etwa einem VPN), lehnt eine Liste ab, die den Speichernden aussperren würde, zählt falsche Passwörter je Person und je Adresse und funktioniert hinter dem Proxy des Servers, sodass man es von außerhalb des Hauses erreicht, ohne den Knoten zu veröffentlichen.
* **Schätzungen sagen es:** ein aus einem Stromeingang gebildeter Kanal trägt `power_estimated`; Studio und Server zeigen seine Leistung als Schätzung (Scheinleistung aus Strom und Nennspannung) und kennzeichnen einen Netzwert, der eine enthält.
* **Optionale verschlüsselte Verbindung zum Broker (TLS):** standardmäßig aus, der Knoten spricht also wie bisher unverschlüsseltes MQTT. Ein Schalter und ein Feld für das Zertifikat der eigenen Zertifizierungsstelle der Anlage (PEM, höchstens 2 KB) auf der Broker-Seite sorgen dafür, dass er sich mit `mqtts://` verbindet, nur dieser Stelle vertraut und prüft, ob das Zertifikat des Brokers für die erreichte Adresse ausgestellt wurde; alles, was nicht genau ein PEM-Zertifikat ist, wird abgelehnt. Durch Host-Tests abgedeckt; der Handshake wurde auf einer Platine noch nicht ausgeführt.
* **Zustandsbericht:** etwa einmal pro Minute veröffentlicht der Knoten, wie lange er läuft und warum er zuletzt neu gestartet ist, seinen freien Speicher, seine Verbindung und sein WLAN-Signal, wie oft er sich mit dem Broker verbunden hat und den letzten protokollierten Fehler (`armor/health/<Knoten>`, der Vertrag `node_health` von ARMOR-COMMON). Der Code, der ihn erzeugt, wird von allen Knoten geteilt und auf einem Computer getestet; auf einer Platine ist er noch nicht gelaufen.
* **Steuerung vom Server aus:** Studio kann den Knoten sauber neu starten, bis zu einer Stunde pausieren (er läuft von selbst weiter), seine Arbeit in den Standby setzen (ohne Ende, über Neustarts hinweg gespeichert, der Knoten bleibt verbunden und antwortet weiter), ihn sich zeigen lassen, einen Diagnose-Schnappschuss anfordern und ihn als in Wartung markieren (`armor/control/<Knoten>/...`, der Vertrag `node_control` von ARMOR-COMMON). Jeder Befehl besteht aus zwei Schritten mit einem Einmal-Token, das zu dieser Operation gehört; nur der Server darf sie am Broker schreiben, und es gibt kein Zurücksetzen auf Werkseinstellungen, kein Löschen und keinen Tiefschlaf. Solange der Knoten pausiert oder im Standby ist, sind alle Relais offen und keines bewegt sich, wer auch fragt (Panel, Broker oder eine Zentrale); die Zähler werden weiter gelesen. Der Code wird von allen Knoten geteilt und auf einem Rechner getestet; auf einer Platine ist er noch nicht gelaufen.
* **Messwerte, die ihre Frische angeben:** Jeder Zählerkanal trägt das Alter seiner Messung und die Zahl der fehlgeschlagenen Anfragen an den Zähler, sodass der Server ein echtes 0 W von einem Zähler unterscheidet, der nicht mehr antwortet; und eine hängende Zähler-Task startet die Platine über den Watchdog neu.
* **Diagnose ohne Kabel:** Nach einem unsauberen Start (Absturz, Watchdog, Spannungseinbruch, fehlgeschlagene Speicherreservierung) nennt der Gesundheitsbericht zehn Minuten lang den Grund; ein Absturz hinterlässt sein Core-Dump, das man im Panel ansehen, herunterladen oder löschen kann; die gespeicherten Einstellungen werden vor jeder Firmware-Installation kopiert und wiederhergestellt, falls ein Update sie beschädigt; und ein Administrator kann den Knoten über den Server mit einem Einmal-Token nach einem Diagnosepaket oder seinem Protokoll fragen.
* **Relais-Verriegelung:** Ein Relais kann ein anderes benennen, das nie gleichzeitig geschlossen sein darf (die zwei Quellen einer Last, die zwei Richtungen eines Motors); das Schließen des einen wird abgelehnt, solange das andere geschlossen ist, vom Panel, vom Broker und von der Zentrale gleichermaßen, und der Knoten öffnet das andere nie von selbst. Auf einem Computer getestet; nicht auf einem Board ausgeführt.
* **Nur lesender Zugriff auf die eigenen Einstellungen der Zähler:** Nach der ersten guten Messung und alle `bus.check_settings_s` (600 s, 0 = nie) liest der Knoten die Holding-Register des PZEM (004T v3: Leistungsschwelle und Adresse; 017: Spannungsschwellen, Adresse und Shunt) im selben Durchgang wie die Messwerte und warnt, wenn die gespeicherte Adresse nicht die verwendete ist; in den Zähler wird nichts geschrieben.
* **Optionales Versiegeln gespeicherter Geheimnisse:** standardmäßig aus (`security.seal_secrets` oder `nodes.nvs_encryption` des Servers). Eingeschaltet werden die Passwörter, Token, Schlüssel, PINs und Passwort-Hashes, die der Knoten speichert, mit ChaCha20-Poly1305 versiegelt, alles andere bleibt lesbar; Ein- und Ausschalten schreibt sie ohne Verlust um, auch wenn der Knoten mittendrin stehen bleibt. Es wird kein eFuse gebrannt und der Flash nicht verschlüsselt: Der Schlüssel liegt in einer Partition desselben Chips und schützt daher **nicht** vor jemandem, der die Platine hat; ein Knoten, der vor dieser Partition installiert wurde, behält seine Geheimnisse unverändert, bis er mit der neuen Partitionstabelle geflasht wird.

## 🧩 Rolle in A.R.M.O.R.

ARMOR-NODE-ELECTRICAL misst das AC- und DC-Netz des Hauses mit seinen Zählern und hält die Regeln zum Umschalten zwischen Quellen. Es veröffentlicht die Messwerte für [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), der Verlauf und Tagesenergie führt und die Alarme auslöst; das Menü *Elektro* und der *Elektro-Designer* von [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) sowie der Elektro-Bildschirm von [ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL) zeigen es. Das Schalten ist entworfen (Scharfstellen, ein Einmal-Token, die Verriegelung, die Bestätigung beider Schütze), aber nicht aktiviert.

| Name | Wie | Wozu |
|---|---|---|
| ARMOR-SERVER | MQTT `armor/electrical/<id>/{state,command,result}` | Empfängt die Kanäle und ordnet eine Umschaltung an (nur wenn das Schalten an drei Stellen aktiviert ist) |
| ARMOR-STUDIO | Menü Elektrik, Electrical Designer | Zeigt die Kanäle und Alarme und öffnet das Panel des Knotens |
| ARMOR-ANDROID-CONTROL | Bluetooth | Richtet den Knoten vor Ort ein und zeigt die Kanäle |
| ARMOR-COMMON | Elektro-Vertrag | Die Nachrichten werden gegen seine Schemas geprüft; die gemeinsame Firmware stammt daraus |

## 🏛️ Architektur

<p align="center">
  <img src="images/architecture.svg" alt="Architektur von ARMOR-NODE-ELECTRICAL: wer es aufruft, was es enthält und wovon es abhängt" width="100%">
</p>

## 🖼️ Screenshots

Aufgenommen mit 1920×1080 auf einem Prüfstand, mit Beispieldaten und ohne eine echte Anlage dahinter.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Übersicht: der Zustand des Knotens auf einen Blick - Netzwerk, Broker, Firmware und was er meldet." width="100%"><br><sub>Übersicht: der Zustand des Knotens auf einen Blick - Netzwerk, Broker, Firmware und was er meldet.</sub></td><td width="50%" valign="top"><img src="images/screenshots/meters.png" alt="Zähler: die Energiezähler und Analogkanäle, die der Knoten liest, jeweils mit Name und Skala." width="100%"><br><sub>Zähler: die Energiezähler und Analogkanäle, die der Knoten liest, jeweils mit Name und Skala.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/readings.png" alt="Messwerte: was der Knoten gerade liest, mit dem Alter jedes Wertes." width="100%"><br><sub>Messwerte: was der Knoten gerade liest, mit dem Alter jedes Wertes.</sub></td><td width="50%" valign="top"><img src="images/screenshots/zigbee.png" alt="Zigbee-Funk: die serielle Brücke zum Koordinatormodul, welcher Client sie nutzen darf, und der Zustand der Brücke." width="100%"><br><sub>Zigbee-Funk: die serielle Brücke zum Koordinatormodul, welcher Client sie nutzen darf, und der Zustand der Brücke.</sub></td></tr>
</table>

## 🔒 Sicherheit und Schutz

| Bedrohung | Abwehr |
|---|---|
| Passwörter im Klartext oder einkompiliert | PBKDF2-HMAC-SHA256 mit 10 000 Runden; kein Passwort in der Firmware (ein Knoten ohne Benutzer geht in die Einrichtung); höchstens 4 Benutzer (Admin oder Betrachter) und 8 bis 64 Zeichen; höchstens 6 Sitzungen, 30 Minuten Leerlauf |
| Einen frisch geflashten Knoten übernehmen | Die Einrichtung braucht einen Startcode: den des Builds (`CONFIG_ARMOR_SETUP_CODE`) oder einen zufälligen im Flash, der auf der USB-Konsole erscheint; mit Flottengeheimnis ist er `HMAC-SHA256(Geheimnis, MAC)`; er ist auch der Schlüssel des Einrichtungs-WLANs (WPA2, nie überbrückt) |
| Das Panel aus dem Internet erreicht | Es antwortet standardmäßig den privaten Netzen eines Hauses (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 und 100.64/10, wo manche VPNs liegen); die Einstellung `web.admin_sources` listet weitere IPv4-Adressen und Netze (8 Einträge, 120 Zeichen); alles außerhalb wird abgewiesen außer der öffentlichen Identität `GET /api/v1/session`; eine Liste, die den Speichernden aussperren würde, wird abgelehnt |
| Das Panel im Klartext über HTTP | HTTPS auf 443 mit einem vom Knoten selbst erzeugten Zertifikat (selbstsigniert, im Flash, SHA-256-Fingerabdruck zum Vergleichen sichtbar); das Panel kann http, https oder beides sein; das Sitzungscookie ist immer HttpOnly und SameSite=Strict (und Secure über TLS) |
| Eine gefälschte Firmware von GitHub | Ein Image wird nur mit passendem SHA-256 und gültiger Ed25519-Signatur mit dem Schlüssel des Maintainers installiert (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` lehnen es ab); ein manueller Upload durch einen Administrator wird nicht geprüft: eine ausdrückliche Grenze |
| Kapern der Bluetooth-Einrichtung | Eine 6-stellige PIN, abgeleitet vom Einrichtungscode, auf der USB-Konsole und für einen Administrator sichtbar; das Telefon muss sie vor dem Schreiben von Einstellungen eingeben; das Einrichtungsfenster dauert `CONFIG_ARMOR_BLE_SETUP_MINUTES` (standardmäßig 10, 1 bis 120) |
| Ein ferner Befehl, der Schaden anrichtet | Zwei Schritte mit einem Einmal-Token, das an diese Operation gebunden ist; nur der Server darf diese Topics im Broker schreiben; eine Pause dauert höchstens eine Stunde; es gibt weder Werksreset noch Löschen noch Tiefschlaf per Fernbefehl |
| Gespeicherte Geheimnisse, gelesen von jemandem mit der Platine | Optional (standardmäßig aus, Schalter `security.seal_secrets` oder `nodes.nvs_encryption` des Servers): ChaCha20-Poly1305 für Passwörter, Token, Schlüssel, PINs und Hashes. Es verschlüsselt nicht den Flash und brennt kein eFuse: Der Schlüssel liegt in einer Partition desselben Chips und schützt daher nicht vor dem, der die Platine hat |
| Die Broker-Verbindung im Klartext | Optionales TLS (`mqtts://`) mit der eigenen Zertifizierungsstelle des Besitzers (PEM bis 2 KB) und Namensprüfung; standardmäßig aus. Der Handshake lief nur in Host-Tests, nicht auf einer Platine |
| Gefährliche Netz- oder Gleichspannung auf der Platine | Der Knoten liest nur die PZEM-Module (geschlossene Module, isolierte serielle Leitung) und führt nie Netzspannung auf seiner Platine; was schaltet, ist eine getrennte Stufe, am Prüfstand mit Lampe und Trenntransformator getestet |
| Zwei Quellen gleichzeitig geschlossen | Die Umschaltverriegelung befiehlt nie beide: Sie braucht erlaubtes Schalten, den kurz zuvor scharf gemachten Knoten und beide Schütze als offen bestätigt während der ganzen Totzeit (`dead_time_ms` standardmäßig 2000); ein Schütz, das nicht zeigt, was befohlen wurde, ist ein Fehler, der bis zur Quittierung bleibt (`kDidNotOpen`, `kDidNotClose`); eine mechanische Verriegelung zwischen den Schützen ist weiterhin Pflicht |
| Ein wiederholter Befehl oder ein Schalten, das niemand wollte | `arm` liefert ein 64-Bit-Einmal-Token, das `close_a` oder `close_b` innerhalb von `arm_window_ms` tragen müssen; Befehle werden nie zurückgehalten; höchstens 4 Schalter; Schalten ist AUS, außer die Firmware erlaubt es (`switching_enabled`), der Server läuft mit `ARMOR_ELECTRICAL_SWITCHING=1` und die Zugriffsliste des Brokers ist für diesen Knoten offen |
| Ein Fehler, verlorene Rückmeldung oder ausgeschaltetes Schalten | Jeder Regler antwortet mit ALLEN Spulen offen (Fehler => offen; ob offen für die Last sicher ist, entscheidet die Installation); der Zustand in der Nachricht ist das, was der Hilfskontakt zeigt, nicht das Befohlene; Relaispaare sind verriegelt, und der Knoten öffnet den Partner nie von sich aus |
| Ein alter Messwert als echt angezeigt | Ein stummer Zähler wird nach 10 s nicht mehr veröffentlicht; jeder Kanal trägt das Alter seines Messwerts und die Zahl fehlgeschlagener Anfragen; eine hängende Zähler-Task startet die Platine per Watchdog neu; alle `bus.check_settings_s` (600 s, 0 = nie) werden die Einstellungen der Zähler gelesen (nur lesend) und eine geänderte Adresse gemeldet |

Netzspannung (230 V AC) und die Gleichspannung einer Batteriebank können töten und Brände auslösen. [docs/SAFETY.md](docs/SAFETY.md) sagt, was der Entwurf voraussetzt; es ist kein Entwurf einer Installation und ersetzt weder die Vorschriften des Landes noch einen qualifizierten Installateur. Heute schaltet nichts hiervon irgendetwas.

## 🔌 Schnittstellen

| Teil | Was sie festlegt |
|---|---|
| MQTT | `armor/electrical/<id>/state` (ein Kanal je Eingang: AC oder DC, Spannung, Strom, Leistung, Energie, Frequenz, Leistungsfaktor, Schalterzustand, Alarm; ARMOR-COMMON-Vertrag), `armor/electrical/<id>/command` und `/result`, `armor/device/<id>/<Relais>/{state,set}`, `armor/health/<id>`, `armor/control/<id>/...` |
| Befehle | `arm`, `close_a`, `close_b` (mit dem Token), `open` und `acknowledge` (löscht einen Fehler) |
| Zähler | Peacefair PZEM-004T v3 (AC) und PZEM-017 (DC) per Modbus RTU, bis zu 16 an einer seriellen Leitung; nur die Leseanfrage wird gebaut (die zum Schreiben von Adresse oder Schwellen gibt es nicht); analoge Eingänge, Kontakte und Relais auf der Basisplatine; eine Zigbee-Brücke ([docs/ZIGBEE.md](docs/ZIGBEE.md)) |

## ⚙️ Konfiguration

| Variable | Standard | Was sie festlegt |
|---|---|---|
| `bus.check_settings_s` | 600 | Sekunden zwischen den rein lesenden Prüfungen der Zählereinstellungen (0 = nie) |
| `dead_time_ms`, `confirm_timeout_ms` | 2000 | Die Totzeit einer Umschaltung und wie lange ein Schütz Zeit hat, das Befohlene zu zeigen |
| `switching_enabled` | off | Schalten ist AUS, außer diese Einstellung, der Server (`ARMOR_ELECTRICAL_SWITCHING=1`) und die Zugriffsliste des Brokers erlauben es |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | Die Einstellungen, die alle Knoten teilen: der Name (leer ergibt `armor-` und die letzten Bytes der MAC), der Einrichtungscode (leer: zufällig), das Flottengeheimnis (16+ Zeichen, nie im Repository), die ersten Broker-Einstellungen, der Herzschlag (2 bis 300 s), die Minuten der Bluetooth-Einrichtung (1 bis 120) und der Reset mit BOOT (8 s in den ersten 30 s) |

## 🚀 Schnellstart

**Voraussetzungen:** ein C++17-Compiler für die Host-Tests; der ESP-IDF-5.5.5-Container zum Bauen des Images; eine ESP32-S3-Platine (Ethernet oder WLAN).

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel without a board
tools/build_node.sh generic s3-wifi                  # the image in the ESP-IDF container (or s3-eth)
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

Siehe den [Entwurf](docs/DESIGN.md), die [Sicherheitshinweise](docs/SAFETY.md), die [Regeln fürs Schalten](docs/SWITCHING.md), die [Protokolle](docs/PROTOCOLS.md) und die [Nachrichten](docs/ELECTRICAL_MESSAGES.md).

## 🧪 Tests

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 Host-Testprogramme und 5 Python-Prüfungen laufen unter `ctest`; 382264 Prüfungen, gezählt von den Host-Programmen, mit `-Werror` gebaut.

Python-Prüfungen: die Panel-Texte in acht Sprachen, die Tasks, die Speicherpfade, die Bluetooth-Sitzung unter ihrer Sperre und die vom Panel aufgerufenen Routen (und in manchen Knoten die HTTP-Codes oder die Glyphen der Schriften)

Die Host-Tests brauchen einen beliebigen C++17-Compiler (Linux, WSL, MSYS2); das Web-Panel wird außerdem in einem echten Browser gegen einen Ersatzknoten geprüft (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

Die meisten Prüfungen sind erschöpfende Durchläufe der Schaltregeln (`test_switch_set` und `test_interlock`: 375 036); Zähler und Schütze sind Ersatz. `test_clock` gibt nur `ok` aus.

## 🏗️ Build und Veröffentlichung

`tools/build_node.sh` baut das Image im ESP-IDF-5.5.5-Container, und die Version trägt das Image, sein SHA-256 und seine Ed25519-Signatur, je eines pro Platine; das Image wird beim ersten Mal per USB-C geschrieben und danach vom Panel aktualisiert.

## 🩺 Fehlerbehebung

| Sie sehen | Weil | Tun Sie |
|---|---|---|
| Du hast das Passwort verloren oder der Knoten tritt dem Netz nicht mehr bei | Ein Knoten in der Einrichtung ohne Adresse nach 90 s öffnet ein eigenes WLAN `ARMOR-SETUP-xxxxxx` (öffne `http://192.168.4.1/`) | Zum Neustart BOOT 8 Sekunden in den ersten 30 Sekunden nach dem Einschalten halten (löscht Einstellungen und Benutzer) oder im Panel *Werksreset* nutzen |
| Ein Update von GitHub wird abgelehnt | Die Version enthält keine Prüfsumme oder Signatur, sie passen nicht (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), hat kein Image für diese Platine (`e_no_asset_for_board`) oder gehört zu einem anderen Projekt (`e_wrong_firmware`) | Der Knoten blieb unverändert; erneut versuchen oder einen Administrator die Datei von Hand hochladen lassen |
| Ein Schalter zeigt einen Fehler | Das Schütz zeigte nicht offen oder geschlossen wie befohlen (`kDidNotOpen`, `kDidNotClose`); es bleibt bis zur Quittierung | Prüfe die Verdrahtung des Hilfskontakts, dann sende `acknowledge` |
| Ein Befehl wird abgelehnt | Schalten ist aus (`switching_enabled` falsch), das Token ist ungültig oder abgelaufen, oder der Knoten war nicht kurz zuvor scharf gemacht | Erneut scharf machen und den Befehl mit dem neuen Token senden |

## 📂 Struktur des Repositorys

<p align="center">
  <img src="images/repository-structure.svg" alt="Repository-Struktur von ARMOR-NODE-ELECTRICAL" width="100%">
</p>

<details>
<summary>Textversion des Baums</summary>

```text
ARMOR-NODE-ELECTRICAL/
├── main/                                die Firmware: eine ESP-IDF-Komponente
├── core/                                die Logik ohne ESP-IDF, am Rechner getestet
├── panel/                               das in die Firmware eingebettete Web-Panel
├── tests/                               Host-Tests und Python-Prüfungen
├── docs/                                12 Anleitungen: Sicherheit, Schalten, Nachrichten, Hardware
├── tools/                               Werkzeuge für Build, Packen und Prüfung
├── .github/                             CI-Workflow sowie Vorlagen für Issues und Pull Requests
├── images/                              Banner, Symbol und die zwei Bilder dieses READMEs
├── build.sh build.bat                   Build über den gemeinsamen Projekt-Workflow
├── test.sh test.bat                     Tests über den gemeinsamen Workflow ausführen
├── run.sh run.bat                       Start über den gemeinsamen Projekt-Workflow
├── armor.project.json                   Manifest: Version, Rolle, Reifegrad, Build-Befehl
├── CHANGELOG.md                         was sich in jeder Version geändert hat
├── SECURITY.md SUPPORT.md LICENSE       Sicherheitsrichtlinie, Support und Lizenz
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   Mitwirkung und Verhaltenskodex
├── CMakeLists.txt                       das ESP-IDF-Projekt
├── partitions.csv                       die Flash-Tabelle
├── sdkconfig.defaults                   die Firmware-Einstellungen
├── sdkconfig.board.s3-eth               die Einstellungen einer Platine
├── sdkconfig.board.s3-wifi              die Einstellungen einer Platine
├── secrets/                             Beispiel der Knotendatei (die echten sind nicht versioniert)
├── build-test.sh build-test.bat         Tests über den gemeinsamen Workflow ausführen
└── .gitattributes .gitignore            Git-Einstellungen
```

</details>

## 🗺️ Roadmap und bekannte Grenzen

**Bekannte Grenzen heute**

* Die Telegramme und Messwerte der PZEM-004T v3 und PZEM-017 werden gegen die dokumentierten Telegramme und aufgezeichnete Beispiele geprüft; noch hat kein Zähler geantwortet.
* Die Seiten für Zähler und Messwerte des Web-Panels wurden in einem Browser gegen einen nachgebildeten Knoten erprobt, nicht auf einer Platine.
* Keine Platine gebaut und kein Zähler oder Schütz angeschlossen: Bisher lief nichts auf echten Geräten.
* Der Einrichtungscode, die Anmeldung mit Begrenzung falscher Passwörter, die Bluetooth-PIN und die signierten Aktualisierungen sind auf dem Host getestet; es gab keine unabhängige Prüfung.
* Die Hash- und Ed25519-Signaturprüfungen und die Rückfalllogik bestehen auf dem Host; eine Aktualisierung über die Luft wurde nie auf einer Platine ausgeführt.
* Die Schaltregeln für Schütze sind nur gegen nachgebildete Schütze getestet; nichts hier hat bisher etwas Echtes geschaltet.

**Als Nächstes**

* Die Zähler, die Schütze und die Schaltregeln am Prüfstand mit Lampe und Trenntransformator erproben, danach eine echte Platine mit dem Update über die Luft.

## 🌍 Sprachen

README und Texte gibt es auf Englisch, Spanisch, Französisch, Italienisch, Deutsch, Chinesisch, Japanisch und europäischem Portugiesisch.

## 🔗 Verwandte Projekte

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) ist ein Perimeter-Sicherheitssystem aus unabhängigen Repositorys. Jedes hat eine eigene Version, eigene Tests und ein eigenes README; hier ist die Familie:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Nachrichtenverträge, Validierer, Konformitätsvektoren und generierte Typen
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Feldknoten-Firmware für ESP32-S3 mit drei Radaren und eigenem Web-Panel
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protokolle für Solar-Wechselrichter und -Batterien und die Nachrichten eines Gateway-Knotens
* **ARMOR-NODE-ELECTRICAL** (dieses Repository) - Elektroknoten: Zähler, die Nachricht der Netzmesswerte und die Regeln fürs Schalten
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Alarmknoten und Alarmzentrale: Zonen, Scharfschalten, Verzögerungen, Sirene und PIN, mit dem Server oder ohne ihn
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - Touch-Panel: der Systemzustand auf einem Wandbildschirm, Scharf- und Quittieren sowie das Zuhause des Sprachassistenten
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - Das lokale Netzwerk: seine Geräte, das Internet und was sich ändert
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Zentraler Koordinator: Telemetrie, Alarme, Geräte, Solarmesswerte und Kameras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Web-Konsole: Kameras, Radar, Alarme, Solarenergie und 2D/3D-Standortdesigner
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Android-Bedienclient mit Live-Radar in 2D/3D
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone-Bedienclient
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Visuelle Inferenzrichtlinie, die ihre Entscheidungen erklärt und nie handelt
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Kamerazonen, Masken und Erkennungsregeln für den Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Offline-Sprachabsichten mit einer nicht fälschbaren Bestätigung
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Gehäuse, Elektronik und die Abnahmematrix am Prüfstand
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Bereitstellung, CM5-Prüfstand, Backup und TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Offline-Telemetriesimulator mit wiederholbaren Fehlern
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Erkennt, installiert und aktualisiert die eigenen Repositorys des Ökosystems
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architektur, Sicherheitsgrundlage und die Fähigkeitsmatrix

## 📚 Dokumentation

* [Sicherheit](docs/SAFETY.md) und [Schalten](docs/SWITCHING.md): was der Entwurf voraussetzt und wie eine Umschaltung abläuft
* [Nachrichten](docs/ELECTRICAL_MESSAGES.md), [Protokolle](docs/PROTOCOLS.md) und [Zigbee](docs/ZIGBEE.md)
* [Entwurf](docs/DESIGN.md), [Hardware](docs/HARDWARE.md), [Basisplatine](docs/BASE_BOARD.md) und [Knoten-Firmware](docs/NODE_FIRMWARE.md)
* [Prüfstandtests](docs/BENCH_TESTS.md), [Einrichtungscode](docs/SETUP_CODE.md) und [Bluetooth-Einrichtung](docs/BLE_PROVISIONING.md)
* [Fähigkeitsmatrix: was belegt ist und was nicht](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Projektkatalog: Versionen und wie die Repositorys voneinander abhängen](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Änderungsverlauf dieses Repositorys](CHANGELOG.md)

## 🤝 Mitwirken und Community

* Führe vor einem Pull Request `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py` aus: Es muss bestehen. Siehe [CONTRIBUTING.md](CONTRIBUTING.md) und den [Verhaltenskodex](CODE_OF_CONDUCT.md).
* Das README gibt es in acht Sprachen, und es wird aus den Quellen in ARMOR-DOCS erzeugt: Eine Textänderung erfolgt dort.
* Eine Änderung an einer Nachricht zwischen Komponenten beginnt in ARMOR-COMMON.
* Beschreibe nichts als auf einer Platine funktionierend, was nur gebaut wurde.
* Melde eine Schwachstelle privat: [SECURITY.md](SECURITY.md). Fragen, Ideen und Meldungen: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTOR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LIZENZ

GPL-3.0-or-later - siehe [LICENSE](LICENSE).
