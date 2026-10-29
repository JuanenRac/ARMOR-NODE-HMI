<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-HMI banner" width="100%">
</p>

# 🖥️ ARMOR-NODE-HMI

<p align="center">
  🇺🇸 <b>English</b> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### The touch panel of the house: a 7-inch screen on the wall that shows the state of the system, arms and disarms it and acknowledges alarms, with a microphone and a speaker for the voice assistant to come (a node of the family on the ESP32-S3-Touch-LCD-7C-BOX, with its own web page for the settings)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 or later">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Language">
  <img src="https://img.shields.io/badge/Board-ESP32--S3--Touch--LCD--7C--BOX-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Checks-1%2C759-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-93122%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.4.0-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturity">
</p>

---

**Honesty check - what runs today:** What does not depend on the board is real and tested on a computer (the settings, what the panel knows of the server and when that is stale, the duress alarm that is never shown, the voice conversation and the long answer of the speech service, the board's pins, the words of the screen in eight languages and the glyphs of its fonts, the release and signature checks: 93,122 checks in 18 test programs) and so is the web page (run against a stand-in in a real browser). **The firmware has been built once (ESP-IDF 5.5.5, not checked by the continuous integration) and has never run on a board:** the screen, the touch, the sound and the link to the server are written from Waveshare's examples for this family of boards and need their first power-up. The speech service (ARMOR-VOICE-AI) answers this panel's requests, there is no wake word: it is push to talk.

---

## 📋 At a glance

| | |
|---|---|
| Version | `0.4.0` |
| Maturity | Functional |
| Role | Wall touch panel for ARMOR-SERVER (family: Clients, parent: ARMOR-SERVER) |
| Stack | C++17, ESP-IDF 5.5.5, LVGL, ESP32-S3 |
| Tests | 93122 checks in 19 host programs and 6 Python checks (`ctest`) |
| Board | Waveshare ESP32-S3-Touch-LCD-7C-BOX (800x480, GT911) |
| Runs on | A Waveshare ESP32-S3-Touch-LCD-7C-BOX; host tests on any computer |
| On real hardware | None yet: the firmware builds (ESP-IDF 5.5.5, not checked by CI) but has not run on the board |
| License | GPL-3.0-or-later |

**Jump to:** [Overview](#-overview) · [Architecture](#-architecture) · [Security](#-security--safety) · [Interfaces](#-interfaces) · [Configuration](#-configuration) · [Quick start](#-quick-start) · [Testing](#-testing) · [Troubleshooting](#-troubleshooting) · [Structure](#-repository-structure) · [Related](#-related-projects)

## 🚦 Project status

Each line is one part of this repository and the light says how far it is today. It is written from what the repository can show (tests, builds, runs), not from what is planned.

🟢 Done and verified · 🟡 Works, with known limits · 🟠 Partly done or not verified yet · 🔴 Not done or not working

| Area | State | What it means here |
|---|---|---|
| Software | 🟠 | The firmware has been built once (ESP-IDF 5.5.5), but the build is not checked by the continuous integration and it has never run on a board. |
| Tests | 🟢 | 93,122 checks in 19 host programs pass (`ctest`): the settings, what the panel knows of the server and when that is stale, the duress alarm that is never shown, the voice exchange, the pins, the words of the screen in eight languages and the glyphs of its fonts. |
| Protocols and data | 🟡 | The exchange with the server and with the speech service ARMOR-VOICE-AI is tested against stand-ins; it is push to talk, with no wake word. |
| Interface | 🟠 | The web page runs against a stand-in in a real browser; the screen, the touch and the sound are written from Waveshare's examples and wait for their first power-up. |
| Hardware | 🔴 | The firmware has not run on the board: nothing has run on real equipment yet. |
| Security | 🟡 | The set-up code, the login limit, the Bluetooth PIN, the duress handling and the signed updates are tested on the host; there has been no independent review. |
| Updates | 🟠 | The hash and signature checks pass on the host; an update over the air has never been run on a board. |
| Integration | 🟡 | The messages are validated against the shared schemas of ARMOR-COMMON; the link to the server and to the speech service has not been tried from a board. |
| Documentation | 🟢 | Eight languages, the firmware guide, the wiring notes and the changelog are up to date. |

## 🎯 Overview

* **A client of ARMOR-SERVER, like the Android app:** it signs in with a user an administrator made for it, asks `GET /api/v1/panel/summary` (a few hundred bytes: the mode, the nodes online and the alarms that need a person) every few seconds and, when the person touches the button and confirms, arms, disarms or acknowledges. If the server stops answering the screen says that what it shows is old and the button is disabled. It listens to the server's live channel (a WebSocket) so a change shows at once, and asks as before when the channel is missing, silent or turned off.
* **The screen (800x480, LVGL):** the mode in the colour of the worst alarm nobody has seen, a large arm/disarm button with a question first, the nodes online, the alarms (the unseen first), a night that dims it, a sleep after a time without touches, a tone and a wake-up when a new alarm arrives, and eight languages with accents, Japanese and Chinese drawn with fonts generated for exactly the characters used.
* **A node like the others:** the same settings store, Wi-Fi, web page in eight languages, Bluetooth set-up from the phone, firmware update with roll-back and HTTPS as ARMOR-NODE-RADAR, ARMOR-NODE-SOLAR and ARMOR-NODE-ELECTRICAL (they share `ARMOR-COMMON/firmware_base`), plus pages for the server, the screen, the sound and the voice; it tells Studio it exists over MQTT so Studio lists it with its page.
* **The voice assistant, push to talk:** a touch opens the microphone, the audio goes only to the speech service whose address is in the settings (off by default), the reply is shown and spoken; the order of the steps, the time limits and what a touch does are a tested state machine, and the panel carries out no command by itself ([the contract](docs/VOICE.md)). The recording goes as raw bytes in a binary request and the spoken answer comes back as bytes; a server that does not accept it is sent the older request.
* **The board:** Waveshare ESP32-S3-Touch-LCD-7C-BOX (32 MB flash, 16 MB PSRAM, a 7-inch RGB screen with a GT911 touch, four microphones, a speaker); the pins come from Waveshare's examples and a test proves that no two functions share a GPIO ([the board](docs/HARDWARE.md)).
* **Signed over-the-air updates:** the node installs from GitHub only an image whose SHA-256 matches and that carries a valid Ed25519 signature of the maintainer (the public key is in the firmware, the private one never leaves the maintainer); a file uploaded by hand by an administrator in the panel is still accepted. The pages and messages of the panel are in eight languages.
* **Bluetooth pairing with a PIN:** the six-digit PIN is derived from the set-up code, printed on the USB console and shown to an administrator in the panel; the phone has to enter it before it may write the node's settings.
* **Not yet:** a first compilation and power-up of the firmware, a wake word, the speech service on the Jetson, and a view of the cameras on the screen.
* **Only the house may use the panel:** the panel answers the private network by default (or the networks its owner lists, such as a VPN), refuses a list that would lock out whoever saves it, counts wrong passwords for each person as well as each address, and works behind the server's proxy so that it can be reached from outside the house without publishing the node.
* **It says what happens:** the screen knows the text, in eight languages, of every alarm code the server raises (network, solar, electrical, alarm zones...), and it tells the server that it is a wall panel, so it is listed as one and does not count as a radar.
* **Optional encrypted link to the broker (TLS):** off by default, so the node talks plain MQTT as before. A switch and a box for the certificate of the installation's own authority (PEM, at most 2 KB) in the Broker page make it connect with `mqtts://`, trust that authority only and check that the broker's certificate was made for the address it reached; anything that is not one PEM certificate is refused. Covered by host tests; the handshake has not been run on a board yet.
* **Health report:** about once a minute the node publishes how long it has been up and why it last restarted, its free memory, its link and Wi-Fi signal, how many times it connected to the broker and the last error it logged (`armor/health/<node>`, the `node_health` contract of ARMOR-COMMON). The code that builds it is shared by every node and tested on a computer; it has not run on a board.
* **Control from the server:** Studio can restart the node cleanly, pause it for up to an hour (it resumes by itself), put its work in standby (no end, kept across restarts, the node stays connected and keeps answering), make it show itself, ask for a diagnostics snapshot and mark it as in maintenance (`armor/control/<node>/...`, the `node_control` contract of ARMOR-COMMON). Every order takes two steps with a one-time token that belongs to that operation, only the server may write them on the broker, and there is no factory reset, no erase and no deep sleep. While the panel is paused or in standby the screen shows a screen of its own that says so and takes no touch; while it is asked to identify itself, a bar blinks at the top (never a sound). The code is shared by every node and tested on a computer; it has not run on a board.
* **Diagnostics without a cable:** after a start that was not clean (a crash, a watchdog, a sag of the supply, a failed allocation) the health report says why for ten minutes; a crash leaves its core dump to look at, download or erase from the panel; the stored settings are copied before every firmware install and recovered if an update damages them; and an administrator can ask the node, through the server, for a diagnostic package or its log with a one-time token.
* **A siren that repeats, and sounds in order:** every sound goes through one priority queue (siren, alarm, voice, confirmation, notice); the siren repeats while critical alarms nobody has acknowledged remain (`audio.siren_repeat_s` 30, `audio.siren_max_repeats` 10) and alarms keep a minimum volume (`audio.critical_min_volume` 40); a voice yields to an alarm.
* **A battery-backed clock:** the board's PCF85063A chip gives the time at start when the node has no real time yet and the chip can be trusted, and it is corrected whenever the time is synchronised or set (`time.rtc_enabled`, on by default).
* **The state of an order, on the screen:** the footer says the order is being sent while it is on its way and that it was confirmed for a few seconds when it went through; a refusal keeps its reason there, as before.
* **Optional sealing of stored secrets:** off by default (`security.seal_secrets`, or the server's `nodes.nvs_encryption`). When on, the passwords, tokens, keys, PINs and password hashes the node stores are kept sealed with ChaCha20-Poly1305 and everything else stays readable; turning it on or off rewrites them with nothing lost, even if the node stops half-way. No eFuse is burned and the flash is not encrypted: the key lives in a partition of the same chip, so it does **not** protect against someone who holds the board, and a node installed before that partition existed keeps its secrets as they were until it is flashed with the new partition table.

## 🧩 Role in A.R.M.O.R.

ARMOR-NODE-HMI is the wall panel: a client of [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), like the Android app, with a 7-inch screen, a microphone and a speaker. It shows the mode, the nodes online and the alarms that need a person, arms and disarms (with the PIN) and acknowledges alarms. It is where the voice assistant ([ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)) will live once its speech service exists, and, like every node, it is set up and updated from [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO).

| Name | How | What for |
|---|---|---|
| ARMOR-SERVER | HTTPS `GET /api/v1/panel/summary`, WebSocket `/api/v1/panel/events` | The summary of the house (modes, alarms, nodes, energy) and its changes; the server decides what an order does |
| ARMOR-VOICE-AI | through the server | The audio of one turn goes to the speech service; the reply is shown and spoken |
| ARMOR-STUDIO | Touch panel menu | Shows the health of the panel and opens its web panel |
| ARMOR-COMMON | firmware base | The common firmware (panel, logins, signed update, Bluetooth set-up) is copied from it |

## 🏛️ Architecture

<p align="center">
  <img src="images/architecture.svg" alt="ARMOR-NODE-HMI architecture: who calls it, what it holds and what it depends on" width="100%">
</p>

## 🖼️ Screenshots

Captured at 1920×1080 on a test bench, with sample data and no real installation behind them.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Overview: the state of the node at a glance - network, broker, firmware and what it reports." width="100%"><br><sub>Overview: the state of the node at a glance - network, broker, firmware and what it reports.</sub></td><td width="50%" valign="top"><img src="images/screenshots/screen.png" alt="Screen and sound: brightness, night hours, volume and which alarms are announced." width="100%"><br><sub>Screen and sound: brightness, night hours, volume and which alarms are announced.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/voice.png" alt="Voice assistant: push to talk, off by default, and the service its audio goes to." width="100%"><br><sub>Voice assistant: push to talk, off by default, and the service its audio goes to.</sub></td><td width="50%" valign="top"><img src="images/screenshots/server.png" alt="Server: the address of the A.R.M.O.R. server, the user of the panel and how often it asks." width="100%"><br><sub>Server: the address of the A.R.M.O.R. server, the user of the panel and how often it asks.</sub></td></tr>
</table>

## 🔒 Security & Safety

| Threat | Defence |
|---|---|
| Passwords in clear or compiled in | PBKDF2-HMAC-SHA256 with 10 000 rounds; no password in the firmware (a node with no user enters set-up); at most 4 users (admin or viewer) and 8 to 64 characters; at most 6 sessions, 30 minutes idle |
| Taking a freshly flashed node | Set-up needs a start-up code: the one of the build (`CONFIG_ARMOR_SETUP_CODE`) or a random one kept in flash and shown on the USB console; with a fleet secret it is `HMAC-SHA256(secret, MAC)`; it is also the key of the set-up Wi-Fi (WPA2, never bridged) |
| The panel reached from the internet | It answers the private networks of a house by default (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 and 100.64/10 where some VPNs live); the setting `web.admin_sources` lists other IPv4 addresses and networks (8 entries, 120 characters); anything outside is refused except the public identity `GET /api/v1/session`; a list that would lock out the person saving it is rejected |
| The panel in clear over HTTP | HTTPS on 443 with a certificate the node makes itself (self-signed, in flash, SHA-256 fingerprint shown to compare); the panel can be http, https or both; the session cookie is always HttpOnly and SameSite=Strict (and Secure over TLS) |
| A fake firmware from GitHub | An image is installed only with a matching SHA-256 and a valid Ed25519 signature made with the maintainer's key (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` refuse it); a manual upload by an administrator is not checked: an explicit limit |
| Hijacking the Bluetooth set-up | A 6-digit PIN derived from the set-up code, shown on the USB console and to an administrator; the phone must type it before writing settings; the set-up window lasts `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 by default, 1 to 120) |
| Stored secrets read by someone who has the board | Optional (off by default, switch `security.seal_secrets` or the server's `nodes.nvs_encryption`): ChaCha20-Poly1305 for passwords, tokens, keys, PINs and hashes. It does not encrypt the flash or burn an eFuse: the key is in a partition of the same chip, so it does not protect against whoever holds the board |
| The broker link in clear | Optional TLS (`mqtts://`) with the owner's own authority (PEM up to 2 KB) and a check of the name; off by default. The handshake has only run in host tests, not on a board |
| The duress alarm shown on the screen | It is never shown (a host test checks it); and what the panel knows of the server is marked as stale when it is old |
| A server that is not the server | The certificate of the server is pinned (`server_pin`), for the requests and for the WebSocket of the live channel; the panel signs in with its own user, created by an administrator, with the role `panel` (it cannot use Studio) |
| Disarming without a PIN | Arming and disarming ask first and take the PIN on the screen (`pin_entry`); the server checks it again |
| Voice audio to an unexpected place | Off by default (`voice.enabled` false); the audio goes only to the service whose address is in the settings (the address is validated and changing the host erases the token); the panel carries out no command by itself and there is no wake word: it is push to talk |
| A siren that nobody hears | One priority queue (siren, alarm, voice, confirmation, notice); the siren repeats while critical alarms are unconfirmed (`audio.siren_repeat_s` 30, 10 to 600, 0 = no repeats; `audio.siren_max_repeats` 10) and the critical volume never goes under `audio.critical_min_volume` (40 %) |

The firmware has been built once and has never run on a board: the screen, the touch, the sound and the link to the server are written from the examples of Waveshare, waiting for the first power-up. Read [SECURITY.md](SECURITY.md) to report a vulnerability.

## 🔌 Interfaces

| Part | What it sets |
|---|---|
| ARMOR-SERVER | `GET /api/v1/panel/summary` every `server.poll_s` (3 s, 1 to 60) only while the live channel is down, and the WebSocket `/api/v1/panel/events` while `server.live_events` is true (the default): the summary arrives when the mode, the alarms, the nodes or the energy change; the panel's own user has the role `panel` |
| Voice | The audio of one turn goes to ARMOR-VOICE-AI through the server (`/api/v1/voice/turn`, with an optional multipart body); `core/voice_session.hpp` is the state machine (order of the steps, time limits, what a touch does); see [docs/VOICE.md](docs/VOICE.md) |
| Hardware | Waveshare ESP32-S3-Touch-LCD-7C-BOX: 7 inch RGB screen 800x480, GT911 touch, four microphones, a speaker, RTC PCF85063A (`time.rtc_enabled`, true by default), 32 MB of flash and 16 MB of PSRAM; a test proves that no function shares a GPIO |

## ⚙️ Configuration

| Variable | Default | What it sets |
|---|---|---|
| `server.poll_s`, `server.live_events` | 3 (1-60), true | The polling period of the summary and the live channel by WebSocket |
| `display.sleep_s` | 300 (0-3600) | Seconds until the screen sleeps (0 = never); the brightness and the night brightness are also settings |
| `audio.critical_min_volume`, `audio.siren_repeat_s`, `audio.siren_max_repeats` | 40 (0-100), 30, 10 | The lowest volume of a critical sound, the siren repeat period (0 or 10 to 600 s) and its maximum repeats |
| `audio.tone_by_source` | by severity | The sound of each alarm source (device, solar, electrical, network, central, other): as its severity says, as a notice, as an alarm, as a siren, no sound, `spoken` (the alarm said out loud) or `custom` (the sound uploaded for that origin), both made by the server and played only if they arrive whole, else the usual tone; a chosen sound plays even below the minimum severity |
| `audio.clip_max_bytes` | 200000 | The most audio, in bytes of 16 kHz 16-bit mono (3200 to 262144), taken from the server for an alarm said out loud or a sound of its own; a longer answer is not played |
| `voice.enabled`, `voice.url`, `voice.token` | false, "", - | The voice service: off by default; the address is at most 160 characters and is validated (`required` or `invalid` otherwise); the token is erased when the host changes |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | The settings every node shares: the name (empty gives `armor-` and the last bytes of the MAC), the set-up code (empty: random), the fleet secret (16+ characters, never in the repository), the first broker settings, the heartbeat (2 to 300 s), the minutes of Bluetooth set-up (1 to 120) and the reset with BOOT (8 s in the first 30 s) |

## 🚀 Quick start

**Requirements:** a C++17 compiler for the host tests; the ESP-IDF 5.5.5 container to build the image; a Waveshare ESP32-S3-Touch-LCD-7C-BOX.

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel of the node without a board
tools/build_node.sh                                   # the image in the ESP-IDF container
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

See the [design](docs/DESIGN.md), the [board](docs/HARDWARE.md), the [voice assistant](docs/VOICE.md), the [firmware guide](docs/NODE_FIRMWARE.md) and the [fonts](docs/FONTS.md).

To build the firmware by hand on GitHub Actions, see [building on GitHub Actions](docs/FIRMWARE_BUILD_ACTIONS.md).

## 🧪 Testing

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 host test programs and 6 Python checks run under `ctest`; 93122 checks counted by the host programs, built with `-Werror`.

Python checks: the panel texts in eight languages, the tasks, the save paths, the Bluetooth session under its lock and the routes the panel calls (and, in some nodes, the HTTP codes or the glyphs of the fonts)

The host tests need any C++17 compiler (Linux, WSL, MSYS2); the web panel is also exercised in a real browser against a stand-in node (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

`check_font_glyphs.py` checks that the fonts hold every glyph of the eight languages (98 rows); `test_clock` prints only `ok`.

## 🏗️ Build & Release

`tools/build_node.sh` builds the image in the ESP-IDF 5.5.5 container and the release carries the image, its SHA-256 and its Ed25519 signature, one per board; the image is written by USB-C the first time and updated from the panel afterwards.

## 🩺 Troubleshooting

| You see | Because | Do |
|---|---|---|
| You lost the password or the node no longer joins the network | A node with set-up and no address after 90 s opens its own Wi-Fi `ARMOR-SETUP-xxxxxx` (open `http://192.168.4.1/`) | To start again hold BOOT for 8 seconds in the first 30 seconds after power-up (erases settings and users), or use *Factory reset* in the panel |
| An update from GitHub is refused | The release has no checksum or signature, they do not match (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), has no image for this board (`e_no_asset_for_board`) or is for another project (`e_wrong_firmware`) | Nothing changed on the node; try again or let an administrator upload the file by hand |
| The voice does not start | `voice.enabled` is off, or `voice.url` is `required` or `invalid` | Turn it on in the panel and give a valid address; changing the host erases the token |
| The footer says sending, then confirmed or the reason | An order is on its way, was confirmed for a few seconds, or was refused and keeps its reason | Read the reason; the server decides, not the panel |

## 📂 Repository Structure

<p align="center">
  <img src="images/repository-structure.svg" alt="Repository structure of ARMOR-NODE-HMI" width="100%">
</p>

<details>
<summary>Text version of the tree</summary>

```text
ARMOR-NODE-HMI/
├── main/                                the firmware: an ESP-IDF component
├── core/                                the logic with no ESP-IDF, tested on a computer
├── panel/                               the web panel embedded in the firmware
├── tests/                               host tests and Python checks
├── docs/                                9 guides: design, voice, fonts, hardware
├── tools/                               build, pack and check tools
├── .github/                             CI workflow, issue and pull-request templates
├── images/                              banner, icon and the two pictures of this README
├── build.sh build.bat                   build through the shared project workflow
├── test.sh test.bat                     run the tests through the shared workflow
├── run.sh run.bat                       start through the shared project workflow
├── armor.project.json                   manifest: version, role, maturity, build command
├── CHANGELOG.md                         what changed in each version
├── SECURITY.md SUPPORT.md LICENSE       security policy, support and license
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   how to contribute and the code of conduct
├── CMakeLists.txt                       the ESP-IDF project
├── partitions.csv                       the flash table
├── sdkconfig.defaults                   the firmware settings
├── sdkconfig.board.lcd7box              the settings of one board
├── secrets/                             example of the node file (the real ones are not versioned)
├── dependencies.lock                    the pinned versions of the ESP-IDF components
├── build-test.sh build-test.bat         run the tests through the shared workflow
└── .gitattributes .gitignore            Git settings
```

</details>

## 🗺️ Roadmap & Known limits

**Known limits today**

* The firmware has been built once (ESP-IDF 5.5.5), but the build is not checked by the continuous integration and it has never run on a board.
* The exchange with the server and with the speech service ARMOR-VOICE-AI is tested against stand-ins; it is push to talk, with no wake word.
* The web page runs against a stand-in in a real browser; the screen, the touch and the sound are written from Waveshare's examples and wait for their first power-up.
* The firmware has not run on the board: nothing has run on real equipment yet.
* The set-up code, the login limit, the Bluetooth PIN, the duress handling and the signed updates are tested on the host; there has been no independent review.
* The hash and signature checks pass on the host; an update over the air has never been run on a board.
* The messages are validated against the shared schemas of ARMOR-COMMON; the link to the server and to the speech service has not been tried from a board.

**Next**

* Do the first full build and the first power-up on the board, then the speech service on the Jetson, a wake word and the camera view on the screen.

## 🌍 Languages

README and texts exist in English, Spanish, French, Italian, German, Chinese, Japanese and European Portuguese.

## 🔗 Related Projects

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) is a perimeter-security system made of independent repositories. Each one has its own version, its own tests and its own README; this is the family:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Message contracts, validators, conformance vectors and generated types
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Field-node firmware for ESP32-S3 with three radars and its own web panel
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Solar inverter and battery protocols and the messages of a gateway node
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Electrical node: meters, the message of the network's readings and the rules for switching
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Alarm node and panel: zones, arming, delays, siren and PIN, with the server or without it
* **ARMOR-NODE-HMI** (this repository) - Touch panel: the state of the system on a wall screen, arming and acknowledging, and the home of the voice assistant
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - The local network: its devices, the internet and what changes
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Central coordinator: telemetry, alarms, devices, solar readings and cameras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Web console: cameras, radar, alarms, solar energy and the 2D/3D site designer
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Android operator client with a live 2D/3D radar
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone operator client
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Visual inference policy that explains its decisions and never actuates
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Camera zones, masks and detection rules for the Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Offline voice intents with a confirmation that cannot be forged
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Enclosures, electronics and the bench acceptance matrix
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Deployment, the CM5 test bench, backup and TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Offline telemetry simulator with repeatable faults
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Detects, installs and updates the ecosystem's own repositories
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architecture, security baseline and the capability matrix

## 📚 Documentation

* [Design](docs/DESIGN.md) and [hardware](docs/HARDWARE.md): what the panel shows and the board
* [Voice](docs/VOICE.md) and [fonts](docs/FONTS.md)
* [Node firmware](docs/NODE_FIRMWARE.md) and [firmware build actions](docs/FIRMWARE_BUILD_ACTIONS.md)
* [Bench tests](docs/BENCH_TESTS.md), [set-up code](docs/SETUP_CODE.md) and [Bluetooth provisioning](docs/BLE_PROVISIONING.md)
* [Capability matrix: what is proven and what is not](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Project catalogue: versions and how the repositories depend on each other](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Changelog of this repository](CHANGELOG.md)

## 🤝 Contributing & Community

* Before a pull request run `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py`: it must pass. See [CONTRIBUTING.md](CONTRIBUTING.md) and the [code of conduct](CODE_OF_CONDUCT.md).
* The README exists in eight languages and is generated from the sources in ARMOR-DOCS: a change to the text is made there.
* A change to a message between components starts in ARMOR-COMMON.
* Do not describe as working on a board what has only been built.
* Report a vulnerability privately: [SECURITY.md](SECURITY.md). Questions, ideas and reports: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTHOR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENSE

GPL-3.0-or-later - see [LICENSE](LICENSE).
