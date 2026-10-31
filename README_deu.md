<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-HMI-Banner" width="100%">
</p>

# 🖥️ ARMOR-NODE-HMI

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

### Das Touch-Panel des Hauses: ein 7-Zoll-Bildschirm an der Wand, der den Systemzustand zeigt, es scharf und unscharf schaltet und Alarme quittiert, mit Mikrofon und Lautsprecher für den kommenden Sprachassistenten (ein Knoten der Familie auf dem ESP32-S3-Touch-LCD-7C-BOX mit eigener Webseite für die Einstellungen)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 oder neuer">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Sprache">
  <img src="https://img.shields.io/badge/Board-ESP32--S3--Touch--LCD--7C--BOX-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Checks-1%2C759-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-93122%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.4.2-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Reifegrad">
</p>

---

**Ehrlichkeitsprüfung - was heute läuft:** Was nicht von der Platine abhängt, ist real und auf einem Computer getestet (die Einstellungen, was das Panel vom Server weiß und wann das veraltet ist, der Zwangsalarm, der nie angezeigt wird, das Sprachgespräch und die lange Antwort des Sprachdienstes, die Pins der Platine, die Texte des Bildschirms in acht Sprachen und die Glyphen seiner Schriften, die Release- und Signaturprüfungen: 30,783 Prüfungen in 18 Testprogrammen), ebenso die Webseite (gegen einen Ersatz in einem echten Browser ausgeführt). **Die Firmware wurde einmal gebaut (ESP-IDF 5.5.5, von der kontinuierlichen Integration nicht geprüft) und lief nie auf einer Platine:** Bildschirm, Touch, Ton und die Verbindung zum Server sind nach Waveshares Beispielen für diese Platinenfamilie geschrieben und brauchen ihr erstes Einschalten. Der Sprachdienst (ARMOR-VOICE-AI) beantwortet die Anfragen dieses Panels; es gibt kein Aktivierungswort: Man tippt zum Sprechen.

---

## 📋 Auf einen Blick

| | |
|---|---|
| Version | `0.4.2` |
| Reifegrad | Funktionsfähig |
| Rolle | Wand-Touchpanel für ARMOR-SERVER (Familie: Clients, übergeordnet: ARMOR-SERVER) |
| Technik | C++17, ESP-IDF 5.5.5, LVGL, ESP32-S3 |
| Tests | 93122 Prüfungen in 19 Host-Programmen und 6 Python-Prüfungen (`ctest`) |
| Platine | Waveshare ESP32-S3-Touch-LCD-7C-BOX (800x480, GT911) |
| Läuft auf | Ein Waveshare ESP32-S3-Touch-LCD-7C-BOX; Host-Tests auf jedem Computer |
| Auf echter Hardware | Noch keiner: die Firmware baut (ESP-IDF 5.5.5, von der CI nicht geprüft), lief aber nicht auf der Platine |
| Lizenz | GPL-3.0-or-later |

**Springe zu:** [Überblick](#-überblick) · [Architektur](#-architektur) · [Sicherheit](#-sicherheit-und-schutz) · [Schnittstellen](#-schnittstellen) · [Konfiguration](#-konfiguration) · [Schnellstart](#-schnellstart) · [Tests](#-tests) · [Probleme](#-fehlerbehebung) · [Struktur](#-struktur-des-repositorys) · [Verwandte](#-verwandte-projekte)

## 🚦 Projektstatus

Jede Zeile ist ein Teil dieses Repositorys, und die Ampel sagt, wie weit er heute ist. Sie beruht auf dem, was das Repository zeigen kann (Tests, Builds, Läufe), nicht auf dem, was geplant ist.

🟢 Fertig und überprüft · 🟡 Funktioniert, mit bekannten Grenzen · 🟠 Teilweise fertig oder noch nicht überprüft · 🔴 Nicht gemacht oder funktioniert nicht

| Bereich | Stand | Was das hier bedeutet |
|---|---|---|
| Software | 🟠 | Die Firmware wurde einmal gebaut (ESP-IDF 5.5.5), der Build wird aber nicht von der kontinuierlichen Integration geprüft, und sie lief nie auf einer Platine. |
| Tests | 🟢 | 93.122 Prüfungen in 19 Host-Programmen bestehen (`ctest`): die Einstellungen, was das Panel vom Server weiß und wann das veraltet ist, der nie angezeigte Zwangsalarm, der Sprachaustausch, die Pins, die Texte des Bildschirms in acht Sprachen und die Glyphen seiner Schriften. |
| Protokolle und Daten | 🟡 | Der Austausch mit dem Server und mit dem Sprachdienst ARMOR-VOICE-AI ist gegen Nachbildungen getestet; es ist Push-to-Talk, ohne Weckwort. |
| Oberfläche | 🟠 | Die Webseite lief gegen eine Nachbildung in einem echten Browser; Bildschirm, Touch und Ton sind nach den Beispielen von Waveshare geschrieben und warten auf das erste Einschalten. |
| Hardware | 🔴 | Die Firmware lief nicht auf der Platine: Bisher lief nichts auf echten Geräten. |
| Sicherheit | 🟡 | Der Einrichtungscode, die Anmeldebegrenzung, die Bluetooth-PIN, der Umgang mit Zwang und die signierten Aktualisierungen sind auf dem Host getestet; es gab keine unabhängige Prüfung. |
| Aktualisierungen | 🟠 | Die Hash- und Signaturprüfungen bestehen auf dem Host; eine Aktualisierung über die Luft wurde nie auf einer Platine ausgeführt. |
| Integration | 🟠 | Die Nachrichten werden gegen die gemeinsamen Schemas von ARMOR-COMMON geprüft; die Verbindung zum Server und zum Sprachdienst wurde nicht von einer Platine aus erprobt. Sein MQTT-Client wird in den Tests gegen einen echten Mosquitto gespielt. |
| Dokumentation | 🟢 | Acht Sprachen, die Firmware-Anleitung, die Verdrahtungshinweise und das Änderungsprotokoll sind aktuell. |

## 🎯 Überblick

* **Ein Client von ARMOR-SERVER wie die Android-App:** Es meldet sich mit einem Benutzer an, den ein Administrator dafür angelegt hat, fragt alle paar Sekunden `GET /api/v1/panel/summary` ab (wenige hundert Byte: Modus, Knoten online und die Alarme, die einen Menschen brauchen) und schaltet scharf, unscharf oder quittiert, wenn die Person die Taste berührt und bestätigt. Antwortet der Server nicht mehr, sagt der Bildschirm, dass das Gezeigte alt ist, und die Taste ist gesperrt. Er hört den Live-Kanal des Servers ab (ein WebSocket), damit eine Änderung sofort erscheint, und fragt wie bisher, wenn der Kanal fehlt, schweigt oder abgeschaltet ist.
* **Der Bildschirm (800x480, LVGL):** der Modus in der Farbe des schlimmsten ungesehenen Alarms, eine große Scharf/Unscharf-Taste mit vorheriger Rückfrage, die Knoten online, die Alarme (die ungesehenen zuerst), eine Nacht, die ihn dimmt, Ruhezustand nach einer Zeit ohne Berührung, ein Ton und Aufwecken bei neuem Alarm sowie acht Sprachen mit Akzenten, Japanisch und Chinesisch mit genau für die verwendeten Zeichen erzeugten Schriften.
* **Ein Knoten wie die anderen:** derselbe Einstellungsspeicher, WLAN, Webseite in acht Sprachen, Bluetooth-Einrichtung vom Handy, Firmware-Update mit Rückfall und HTTPS wie ARMOR-NODE-RADAR, ARMOR-NODE-SOLAR und ARMOR-NODE-ELECTRICAL (sie teilen `ARMOR-COMMON/firmware_base`), dazu Seiten für Server, Bildschirm, Ton und Sprache; es meldet sich per MQTT bei Studio an, damit Studio es mit seiner Seite auflistet.
* **Der Sprachassistent, Push-to-talk:** Eine Berührung öffnet das Mikrofon, das Audio geht nur an den Sprachdienst, dessen Adresse in den Einstellungen steht (standardmäßig aus), die Antwort wird angezeigt und gesprochen; Reihenfolge der Schritte, Zeitlimits und die Wirkung einer Berührung sind ein getesteter Zustandsautomat, und das Panel führt keinen Befehl von selbst aus ([der Vertrag](docs/VOICE.md)). Die Aufnahme geht als Rohbytes in einer Binäranfrage hinaus und die gesprochene Antwort kommt als Bytes zurück; einem Server, der das nicht annimmt, wird die ältere Anfrage geschickt.
* **Das Board:** Waveshare ESP32-S3-Touch-LCD-7C-BOX (32 MB Flash, 16 MB PSRAM, 7-Zoll-RGB-Bildschirm mit GT911-Touch, vier Mikrofone, ein Lautsprecher); die Pins stammen aus Waveshares Beispielen, und ein Test beweist, dass keine zwei Funktionen einen GPIO teilen ([das Board](docs/HARDWARE.md)).
* **Signierte Over-the-Air-Updates:** der Knoten installiert von GitHub nur ein Abbild, dessen SHA-256 stimmt und das eine gültige Ed25519-Signatur des Betreuers trägt (der öffentliche Schlüssel steckt in der Firmware, der private verlässt den Betreuer nie); eine von einem Administrator im Panel von Hand hochgeladene Datei wird weiterhin akzeptiert. Seiten und Meldungen des Panels sind in acht Sprachen.
* **Bluetooth-Kopplung mit PIN:** die sechsstellige PIN wird aus dem Einrichtungscode abgeleitet, auf der USB-Konsole ausgegeben und einem Administrator im Panel angezeigt; das Telefon muss sie eingeben, bevor es die Einstellungen des Knotens schreiben darf.
* **Noch nicht:** eine erste Kompilierung und ein erster Start der Firmware, ein Aktivierungswort, der Sprachdienst auf dem Jetson und eine Kameraansicht auf dem Bildschirm.
* **Nur das Haus darf das Panel nutzen:** Das Panel antwortet standardmäßig dem privaten Netz (oder den Netzen, die der Besitzer nennt, etwa einem VPN), lehnt eine Liste ab, die den Speichernden aussperren würde, zählt falsche Passwörter je Person und je Adresse und funktioniert hinter dem Proxy des Servers, sodass man es von außerhalb des Hauses erreicht, ohne den Knoten zu veröffentlichen.
* **Es sagt, was passiert:** der Bildschirm kennt den Text jedes Alarmcodes des Servers (Netzwerk, Solar, Elektrik, Alarmzonen...) in acht Sprachen und meldet dem Server, dass er ein Wandpanel ist; so wird er als solches gelistet und zählt nicht als Radar.
* **Optionale verschlüsselte Verbindung zum Broker (TLS):** standardmäßig aus, der Knoten spricht also wie bisher unverschlüsseltes MQTT. Ein Schalter und ein Feld für das Zertifikat der eigenen Zertifizierungsstelle der Anlage (PEM, höchstens 2 KB) auf der Broker-Seite sorgen dafür, dass er sich mit `mqtts://` verbindet, nur dieser Stelle vertraut und prüft, ob das Zertifikat des Brokers für die erreichte Adresse ausgestellt wurde; alles, was nicht genau ein PEM-Zertifikat ist, wird abgelehnt. Durch Host-Tests abgedeckt; der Handshake wurde auf einer Platine noch nicht ausgeführt.
* **Zustandsbericht:** etwa einmal pro Minute veröffentlicht der Knoten, wie lange er läuft und warum er zuletzt neu gestartet ist, seinen freien Speicher, seine Verbindung und sein WLAN-Signal, wie oft er sich mit dem Broker verbunden hat und den letzten protokollierten Fehler (`armor/health/<Knoten>`, der Vertrag `node_health` von ARMOR-COMMON). Der Code, der ihn erzeugt, wird von allen Knoten geteilt und auf einem Computer getestet; auf einer Platine ist er noch nicht gelaufen.
* **Steuerung vom Server aus:** Studio kann den Knoten sauber neu starten, bis zu einer Stunde pausieren (er läuft von selbst weiter), seine Arbeit in den Standby setzen (ohne Ende, über Neustarts hinweg gespeichert, der Knoten bleibt verbunden und antwortet weiter), ihn sich zeigen lassen, einen Diagnose-Schnappschuss anfordern und ihn als in Wartung markieren (`armor/control/<Knoten>/...`, der Vertrag `node_control` von ARMOR-COMMON). Jeder Befehl besteht aus zwei Schritten mit einem Einmal-Token, das zu dieser Operation gehört; nur der Server darf sie am Broker schreiben, und es gibt kein Zurücksetzen auf Werkseinstellungen, kein Löschen und keinen Tiefschlaf. Solange das Panel pausiert oder im Standby ist, zeigt der Bildschirm eine eigene Anzeige, die das sagt, und nimmt keine Berührung an; bei der Aufforderung, sich zu melden, blinkt oben ein Balken (nie ein Ton). Der Code wird von allen Knoten geteilt und auf einem Rechner getestet; auf einer Platine ist er noch nicht gelaufen.
* **Diagnose ohne Kabel:** Nach einem unsauberen Start (Absturz, Watchdog, Spannungseinbruch, fehlgeschlagene Speicherreservierung) nennt der Gesundheitsbericht zehn Minuten lang den Grund; ein Absturz hinterlässt sein Core-Dump, das man im Panel ansehen, herunterladen oder löschen kann; die gespeicherten Einstellungen werden vor jeder Firmware-Installation kopiert und wiederhergestellt, falls ein Update sie beschädigt; und ein Administrator kann den Knoten über den Server mit einem Einmal-Token nach einem Diagnosepaket oder seinem Protokoll fragen.
* **Eine Sirene, die sich wiederholt, und Töne in Reihenfolge:** Jeder Ton läuft durch eine Prioritätswarteschlange (Sirene, Alarm, Stimme, Bestätigung, Hinweis); die Sirene wiederholt sich, solange es unquittierte kritische Alarme gibt (`audio.siren_repeat_s` 30, `audio.siren_max_repeats` 10), und Alarme behalten eine Mindestlautstärke (`audio.critical_min_volume` 40); die Stimme weicht einem Alarm.
* **Eine batteriegepufferte Uhr:** Der Chip PCF85063A der Platine liefert beim Start die Uhrzeit, wenn der Knoten noch keine echte Zeit hat und der Chip vertrauenswürdig ist, und wird bei jeder Synchronisierung oder Zeiteinstellung korrigiert (`time.rtc_enabled`, standardmäßig an).
* **Der Zustand eines Befehls auf dem Bildschirm:** Die Fußzeile zeigt, dass der Befehl gesendet wird, solange er unterwegs ist, und einige Sekunden lang, dass er bestätigt wurde; eine Ablehnung behält dort wie bisher ihren Grund.
* **Optionales Versiegeln gespeicherter Geheimnisse:** standardmäßig aus (`security.seal_secrets` oder `nodes.nvs_encryption` des Servers). Eingeschaltet werden die Passwörter, Token, Schlüssel, PINs und Passwort-Hashes, die der Knoten speichert, mit ChaCha20-Poly1305 versiegelt, alles andere bleibt lesbar; Ein- und Ausschalten schreibt sie ohne Verlust um, auch wenn der Knoten mittendrin stehen bleibt. Es wird kein eFuse gebrannt und der Flash nicht verschlüsselt: Der Schlüssel liegt in einer Partition desselben Chips und schützt daher **nicht** vor jemandem, der die Platine hat; ein Knoten, der vor dieser Partition installiert wurde, behält seine Geheimnisse unverändert, bis er mit der neuen Partitionstabelle geflasht wird.

## 🧩 Rolle in A.R.M.O.R.

ARMOR-NODE-HMI ist das Wandpanel: ein Client von [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), wie die Android-App, mit einem 7-Zoll-Bildschirm, Mikrofon und Lautsprecher. Es zeigt den Modus, die Knoten online und die Alarme, die eine Person brauchen, schaltet scharf und unscharf (mit der PIN) und quittiert Alarme. Hier wird der Sprachassistent ([ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)) wohnen, sobald sein Sprachdienst existiert, und wie jeder Knoten wird es von [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) aus eingerichtet und aktualisiert.

| Name | Wie | Wozu |
|---|---|---|
| ARMOR-SERVER | HTTPS `GET /api/v1/panel/summary`, WebSocket `/api/v1/panel/events` | Die Zusammenfassung des Hauses (Modi, Alarme, Knoten, Energie) und ihre Änderungen; der Server entscheidet, was ein Befehl bewirkt |
| ARMOR-VOICE-AI | über den Server | Das Audio eines Turns geht an den Sprachdienst; die Antwort wird angezeigt und gesprochen |
| ARMOR-STUDIO | Menü Touchpanel | Zeigt die Gesundheit des Panels und öffnet sein Web-Panel |
| ARMOR-COMMON | Firmware-Basis | Die gemeinsame Firmware (Panel, Anmeldungen, signiertes Update, Bluetooth-Einrichtung) stammt daraus |

## 🏛️ Architektur

<p align="center">
  <img src="images/architecture.svg" alt="Architektur von ARMOR-NODE-HMI: wer es aufruft, was es enthält und wovon es abhängt" width="100%">
</p>

## 🖼️ Screenshots

Aufgenommen mit 1920×1080 auf einem Prüfstand, mit Beispieldaten und ohne eine echte Anlage dahinter.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Übersicht: der Zustand des Knotens auf einen Blick - Netzwerk, Broker, Firmware und was er meldet." width="100%"><br><sub>Übersicht: der Zustand des Knotens auf einen Blick - Netzwerk, Broker, Firmware und was er meldet.</sub></td><td width="50%" valign="top"><img src="images/screenshots/screen.png" alt="Bildschirm und Ton: Helligkeit, Nachtstunden, Lautstärke und welche Alarme angesagt werden." width="100%"><br><sub>Bildschirm und Ton: Helligkeit, Nachtstunden, Lautstärke und welche Alarme angesagt werden.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/voice.png" alt="Sprachassistent: Drücken zum Sprechen, standardmäßig aus, und der Dienst, an den sein Audio geht." width="100%"><br><sub>Sprachassistent: Drücken zum Sprechen, standardmäßig aus, und der Dienst, an den sein Audio geht.</sub></td><td width="50%" valign="top"><img src="images/screenshots/server.png" alt="Server: die Adresse des A.R.M.O.R.-Servers, der Benutzer des Panels und wie oft es nachfragt." width="100%"><br><sub>Server: die Adresse des A.R.M.O.R.-Servers, der Benutzer des Panels und wie oft es nachfragt.</sub></td></tr>
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
| Gespeicherte Geheimnisse, gelesen von jemandem mit der Platine | Optional (standardmäßig aus, Schalter `security.seal_secrets` oder `nodes.nvs_encryption` des Servers): ChaCha20-Poly1305 für Passwörter, Token, Schlüssel, PINs und Hashes. Es verschlüsselt nicht den Flash und brennt kein eFuse: Der Schlüssel liegt in einer Partition desselben Chips und schützt daher nicht vor dem, der die Platine hat |
| Die Broker-Verbindung im Klartext | Optionales TLS (`mqtts://`) mit der eigenen Zertifizierungsstelle des Besitzers (PEM bis 2 KB) und Namensprüfung; standardmäßig aus. Der Handshake lief nur in Host-Tests, nicht auf einer Platine |
| Der Zwangsalarm auf dem Bildschirm | Er wird nie angezeigt (ein Host-Test prüft es); und was das Panel vom Server weiß, wird als veraltet markiert, wenn es alt ist |
| Ein Server, der nicht der Server ist | Das Zertifikat des Servers ist festgelegt (`server_pin`), für die Anfragen und für den WebSocket des Live-Kanals; das Panel meldet sich mit einem eigenen, von einem Administrator angelegten Benutzer mit der Rolle `panel` an (es kann Studio nicht benutzen) |
| Unscharfschalten ohne PIN | Scharf- und Unscharfschalten fragen zuerst und nehmen die PIN am Bildschirm entgegen (`pin_entry`); der Server prüft sie erneut |
| Sprachaudio an einen unerwarteten Ort | Standardmäßig aus (`voice.enabled` falsch); das Audio geht nur an den Dienst, dessen Adresse in den Einstellungen steht (die Adresse wird geprüft, und ein Hostwechsel löscht das Token); das Panel führt von sich aus keinen Befehl aus, und es gibt kein Aktivierungswort: Es ist Push-to-Talk |
| Eine Sirene, die niemand hört | Eine einzige Prioritätswarteschlange (Sirene, Alarm, Sprache, Bestätigung, Hinweis); die Sirene wiederholt sich, solange kritische Alarme unquittiert sind (`audio.siren_repeat_s` 30, 10 bis 600, 0 = keine Wiederholung; `audio.siren_max_repeats` 10), und die kritische Lautstärke sinkt nie unter `audio.critical_min_volume` (40 %) |

Die Firmware wurde einmal gebaut und lief nie auf einer Platine: Bildschirm, Touch, Ton und Serververbindung sind nach den Beispielen von Waveshare geschrieben und warten auf das erste Einschalten. Lies [SECURITY.md](SECURITY.md), um eine Schwachstelle zu melden.

## 🔌 Schnittstellen

| Teil | Was sie festlegt |
|---|---|
| ARMOR-SERVER | `GET /api/v1/panel/summary` alle `server.poll_s` (3 s, 1 bis 60) nur solange der Live-Kanal ausgefallen ist, und der WebSocket `/api/v1/panel/events`, solange `server.live_events` wahr ist (Standard): Die Zusammenfassung kommt, wenn sich Modus, Alarme, Knoten oder Energie ändern; der eigene Benutzer des Panels hat die Rolle `panel` |
| Sprache | Das Audio eines Turns geht über den Server an ARMOR-VOICE-AI (`/api/v1/voice/turn`, mit optionalem Multipart-Body); `core/voice_session.hpp` ist die Zustandsmaschine (Reihenfolge der Schritte, Zeitgrenzen, was eine Berührung bewirkt); siehe [docs/VOICE.md](docs/VOICE.md) |
| Hardware | Waveshare ESP32-S3-Touch-LCD-7C-BOX: 7-Zoll-RGB-Bildschirm 800x480, GT911-Touch, vier Mikrofone, ein Lautsprecher, RTC PCF85063A (`time.rtc_enabled`, standardmäßig wahr), 32 MB Flash und 16 MB PSRAM; ein Test belegt, dass keine Funktion ein GPIO teilt |

## ⚙️ Konfiguration

| Variable | Standard | Was sie festlegt |
|---|---|---|
| `server.poll_s`, `server.live_events` | 3 (1-60), true | Die Abfrageperiode der Zusammenfassung und der Live-Kanal per WebSocket |
| `display.sleep_s` | 300 (0-3600) | Sekunden bis der Bildschirm schläft (0 = nie); Helligkeit und Nachthelligkeit sind ebenfalls Einstellungen |
| `audio.critical_min_volume`, `audio.siren_repeat_s`, `audio.siren_max_repeats` | 40 (0-100), 30, 10 | Die niedrigste Lautstärke eines kritischen Tons, die Wiederholperiode der Sirene (0 oder 10 bis 600 s) und ihre maximalen Wiederholungen |
| `audio.tone_by_source` | nach Schweregrad | Der Ton jeder Alarmquelle (Gerät, Solar, Elektrik, Netz, Zentrale, andere): wie es der Schweregrad vorgibt, als Hinweis, als Alarm, als Sirene ohne Ton, `spoken` (der Alarm laut gesprochen) oder `custom` (der für diese Herkunft hochgeladene Ton), beides vom Server erzeugt und nur bei vollständigem Eintreffen abgespielt, sonst der übliche Ton; ein gewählter Ton erklingt auch unterhalb des Mindestschweregrads |
| `audio.clip_max_bytes` | 200000 | Höchstmenge Audio in Bytes (16 kHz, 16 Bit, Mono; 3200 bis 262144), die der Server für einen laut gesprochenen Alarm oder einen eigenen Ton liefern darf; eine längere Antwort wird nicht abgespielt |
| `voice.enabled`, `voice.url`, `voice.token` | false, "", - | Der Sprachdienst: standardmäßig aus; die Adresse hat höchstens 160 Zeichen und wird geprüft (sonst `required` oder `invalid`); das Token wird beim Hostwechsel gelöscht |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | Die Einstellungen, die alle Knoten teilen: der Name (leer ergibt `armor-` und die letzten Bytes der MAC), der Einrichtungscode (leer: zufällig), das Flottengeheimnis (16+ Zeichen, nie im Repository), die ersten Broker-Einstellungen, der Herzschlag (2 bis 300 s), die Minuten der Bluetooth-Einrichtung (1 bis 120) und der Reset mit BOOT (8 s in den ersten 30 s) |
| `system.login_limit_per_minute` | 30 (0-120) | Falsche Passwörter pro Minute, bevor das Panel die Anmeldung für alle sperrt (0 = kein globales Limit); in Studio (eine Schaltfläche an jedem Knoten) oder mit `POST /api/v1/admin/nodes/settings` lesen und ändern, wobei die Panel-Anmeldung einmal benutzt und nie gespeichert wird |

## 🚀 Schnellstart

**Voraussetzungen:** ein C++17-Compiler für die Host-Tests; der ESP-IDF-5.5.5-Container zum Bauen des Images; ein Waveshare ESP32-S3-Touch-LCD-7C-BOX.

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel of the node without a board
tools/build_node.sh                                   # the image in the ESP-IDF container
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

Siehe den [Entwurf](docs/DESIGN.md), das [Board](docs/HARDWARE.md), den [Sprachassistenten](docs/VOICE.md), die [Firmware-Anleitung](docs/NODE_FIRMWARE.md) und die [Schriften](docs/FONTS.md).

Um die Firmware von Hand in GitHub Actions zu bauen, siehe [Bauen in GitHub Actions](docs/FIRMWARE_BUILD_ACTIONS.md).

## 🧪 Tests

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 Host-Testprogramme und 6 Python-Prüfungen laufen unter `ctest`; 93122 Prüfungen, gezählt von den Host-Programmen, mit `-Werror` gebaut.

Python-Prüfungen: die Panel-Texte in acht Sprachen, die Tasks, die Speicherpfade, die Bluetooth-Sitzung unter ihrer Sperre und die vom Panel aufgerufenen Routen (und in manchen Knoten die HTTP-Codes oder die Glyphen der Schriften)

Die Host-Tests brauchen einen beliebigen C++17-Compiler (Linux, WSL, MSYS2); das Web-Panel wird außerdem in einem echten Browser gegen einen Ersatzknoten geprüft (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

`check_font_glyphs.py` prüft, dass die Schriften alle Glyphen der acht Sprachen enthalten (98 Zeilen); `test_clock` gibt nur `ok` aus.

## 🏗️ Build und Veröffentlichung

`tools/build_node.sh` baut das Image im ESP-IDF-5.5.5-Container, und die Version trägt das Image, sein SHA-256 und seine Ed25519-Signatur, je eines pro Platine; das Image wird beim ersten Mal per USB-C geschrieben und danach vom Panel aktualisiert.

## 🩺 Fehlerbehebung

| Sie sehen | Weil | Tun Sie |
|---|---|---|
| Du hast das Passwort verloren oder der Knoten tritt dem Netz nicht mehr bei | Ein Knoten in der Einrichtung ohne Adresse nach 90 s öffnet ein eigenes WLAN `ARMOR-SETUP-xxxxxx` (öffne `http://192.168.4.1/`) | Zum Neustart BOOT 8 Sekunden in den ersten 30 Sekunden nach dem Einschalten halten (löscht Einstellungen und Benutzer) oder im Panel *Werksreset* nutzen |
| Ein Update von GitHub wird abgelehnt | Die Version enthält keine Prüfsumme oder Signatur, sie passen nicht (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), hat kein Image für diese Platine (`e_no_asset_for_board`) oder gehört zu einem anderen Projekt (`e_wrong_firmware`) | Der Knoten blieb unverändert; erneut versuchen oder einen Administrator die Datei von Hand hochladen lassen |
| Die Sprache startet nicht | `voice.enabled` ist aus, oder `voice.url` ist `required` oder `invalid` | Schalte es im Panel ein und gib eine gültige Adresse an; ein Hostwechsel löscht das Token |
| Die Fußzeile sagt „sendet“, dann „bestätigt“ oder den Grund | Ein Befehl ist unterwegs, wurde einige Sekunden bestätigt oder abgelehnt und behält seinen Grund | Lies den Grund; der Server entscheidet, nicht das Panel |

## 📂 Struktur des Repositorys

<p align="center">
  <img src="images/repository-structure.svg" alt="Repository-Struktur von ARMOR-NODE-HMI" width="100%">
</p>

<details>
<summary>Textversion des Baums</summary>

```text
ARMOR-NODE-HMI/
├── main/                                die Firmware: eine ESP-IDF-Komponente
├── core/                                die Logik ohne ESP-IDF, am Rechner getestet
├── panel/                               das in die Firmware eingebettete Web-Panel
├── tests/                               Host-Tests und Python-Prüfungen
├── docs/                                9 Anleitungen: Entwurf, Sprache, Schriften, Hardware
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
├── sdkconfig.board.lcd7box              die Einstellungen einer Platine
├── secrets/                             Beispiel der Knotendatei (die echten sind nicht versioniert)
├── dependencies.lock                    die festgelegten Versionen der ESP-IDF-Komponenten
├── build-test.sh build-test.bat         Tests über den gemeinsamen Workflow ausführen
└── .gitattributes .gitignore            Git-Einstellungen
```

</details>

## 🗺️ Roadmap und bekannte Grenzen

**Bekannte Grenzen heute**

* Die Firmware wurde einmal gebaut (ESP-IDF 5.5.5), der Build wird aber nicht von der kontinuierlichen Integration geprüft, und sie lief nie auf einer Platine.
* Der Austausch mit dem Server und mit dem Sprachdienst ARMOR-VOICE-AI ist gegen Nachbildungen getestet; es ist Push-to-Talk, ohne Weckwort.
* Die Webseite lief gegen eine Nachbildung in einem echten Browser; Bildschirm, Touch und Ton sind nach den Beispielen von Waveshare geschrieben und warten auf das erste Einschalten.
* Die Firmware lief nicht auf der Platine: Bisher lief nichts auf echten Geräten.
* Der Einrichtungscode, die Anmeldebegrenzung, die Bluetooth-PIN, der Umgang mit Zwang und die signierten Aktualisierungen sind auf dem Host getestet; es gab keine unabhängige Prüfung.
* Die Hash- und Signaturprüfungen bestehen auf dem Host; eine Aktualisierung über die Luft wurde nie auf einer Platine ausgeführt.
* Die Nachrichten werden gegen die gemeinsamen Schemas von ARMOR-COMMON geprüft; die Verbindung zum Server und zum Sprachdienst wurde nicht von einer Platine aus erprobt.

**Als Nächstes**

* Den ersten vollständigen Build und das erste Einschalten auf der Platine machen, danach den Sprachdienst auf dem Jetson, ein Aktivierungswort und die Kameraansicht auf dem Bildschirm.

## 🌍 Sprachen

README und Texte gibt es auf Englisch, Spanisch, Französisch, Italienisch, Deutsch, Chinesisch, Japanisch und europäischem Portugiesisch.

## 🔗 Verwandte Projekte

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) ist ein Perimeter-Sicherheitssystem aus unabhängigen Repositorys. Jedes hat eine eigene Version, eigene Tests und ein eigenes README; hier ist die Familie:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Nachrichtenverträge, Validierer, Konformitätsvektoren und generierte Typen
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Feldknoten-Firmware für ESP32-S3 mit drei Radaren und eigenem Web-Panel
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protokolle für Solar-Wechselrichter und -Batterien und die Nachrichten eines Gateway-Knotens
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Elektroknoten: Zähler, die Nachricht der Netzmesswerte und die Regeln fürs Schalten
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Alarmknoten und Alarmzentrale: Zonen, Scharfschalten, Verzögerungen, Sirene und PIN, mit dem Server oder ohne ihn
* **ARMOR-NODE-HMI** (dieses Repository) - Touch-Panel: der Systemzustand auf einem Wandbildschirm, Scharf- und Quittieren sowie das Zuhause des Sprachassistenten
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

* [Entwurf](docs/DESIGN.md) und [Hardware](docs/HARDWARE.md): was das Panel zeigt und die Platine
* [Sprache](docs/VOICE.md) und [Schriften](docs/FONTS.md)
* [Knoten-Firmware](docs/NODE_FIRMWARE.md) und [Build-Aktionen](docs/FIRMWARE_BUILD_ACTIONS.md)
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
