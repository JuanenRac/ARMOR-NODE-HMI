# ARMOR-NODE-HMI - design

ARMOR-NODE-HMI is the touch panel of an A.R.M.O.R. installation: a 7-inch screen on a wall that shows the state of the system, lets a person arm, disarm and acknowledge from there, and
is the place where the voice assistant will live. It is two things at once, and the design keeps them apart:

1. **A node of the family.** Like ARMOR-NODE-RADAR, ARMOR-NODE-SOLAR and ARMOR-NODE-ELECTRICAL it keeps its settings and its users in flash, joins the Wi-Fi, has its own web page for the
   settings, can be set up from a phone over Bluetooth, can be updated from its page (with a roll-back if the new firmware does not come up) and tells ARMOR-SERVER that it
   exists. All of that is `ARMOR-COMMON/firmware_base`, synchronised into this repository (`python ../ARMOR-COMMON/tools/sync_firmware_base.py check`).
2. **A client of ARMOR-SERVER**, like the Android app and Studio. It signs in with a user an administrator made for it and asks the server what it needs to draw.

The panel decides nothing. What it shows is what the server says; arming and disarming are the person's, confirmed on the screen, and the server's (it authenticates and authorises
every order exactly as it does for the Android app).

Disarming asks for a PIN on the screen. It is the PIN of the panel's own account on the server (an administrator sets it in Studio, Configuration > Users); the server checks it, locks the account after three wrong ones and, if the duress PIN is typed, disarms while it raises a critical alarm in silence. The digits are drawn as dots and wiped from memory once sent.

## What is tested where

| Part | Where | How |
| --- | --- | --- |
| The settings (`core/hmi_config.hpp`) | the computer | `tests/test_config.cpp`: defaults, the link to the server, the screen, the voice, the stored document, passwords that never leave |
| What the panel knows of the server (`core/server_view.hpp`) | the computer | `tests/test_view.cpp`: the summary, the state of the link, when data is stale, what is new, the colour of the screen, the night brightness |
| The voice conversation (`core/voice_session.hpp`, `core/voice_answer.hpp`) | the computer | `tests/test_voice.cpp`: the order of the steps, the time limits, what a touch does, the answer of the service with its long audio |
| The pins of the board (`core/board_s3.hpp`) | the computer | `tests/test_board.cpp` and a `static_assert`: no two functions share a GPIO and none is a pin the chip keeps |
| The words of the screen (`core/screen_text.hpp`) | the computer | `tests/test_board.cpp`: every row in eight languages; `tests/check_font_glyphs.py`: every character the screen can draw has its glyph in the font of its language |
| The web page (`panel/`) | a real browser | `node tools/panel_browser_test.mjs` against `tools/panel_mock.mjs`: every page in eight languages |
| The firmware (`main/`) | **nowhere yet** | it has built with ESP-IDF 5.5.5 (`tools/build_node.sh generic`, or `idf.py` on the machine; the continuous integration does not build it) and has never been run on a board |

## The link to the server

```
panel                                     ARMOR-SERVER
  | POST /api/v1/studio/session {user, password}        -> 201 + cookie
  | GET  /api/v1/panel/summary  (every poll_s seconds)  -> {mode, nodes{online,total}, alarms{active, unacknowledged, items[<=6]}}
  | WS   /api/v1/panel/events   (the same summary when it changes, and at least every heartbeat_ms)  -> {type:"summary", heartbeat_ms, summary:{...}}
  | POST /api/v1/mode {"mode":"armed"|"disarmed","pin":"..."}  (after the person confirms on the screen; a disarm carries the PIN typed on the keypad, which the server checks)
  | POST /api/v1/alarms/acknowledge {"pin":"..."}        (the button "Acknowledge all": after the keypad, by default; see below)
```

`GET /api/v1/panel/summary` was added for small screens (ARMOR-SERVER 0.3.7): a few hundred bytes whatever the installation holds, with the alarms nobody has acknowledged first.
**The live channel.** With `server.live_events` on (the default) the panel opens a WebSocket at `/api/v1/panel/events` with the same session cookie and the same trust as its requests (the fingerprint, the pinned certificate or the usual authorities for `wss://`). The server sends the summary when the socket opens, whenever something in it changes, and again every few seconds (`heartbeat_ms` in the frame says how often), and nothing else: the channel is read-only and carries no more than the panel could ask for. While frames arrive the panel does not ask; when they do not (the channel has not connected yet, the server is older and has none, the connection dropped, the frames stopped for three heartbeats) it asks every `poll_s` seconds as always, and tries the socket again after a wait that grows from five seconds to five minutes. The summary counts as stale after three heartbeats without news while listening, and after four polls while asking. An administrator turns the channel off on the server (`panel.live_events`): the socket is closed with the code 4403 and the panel does not try it again for ten minutes; a close with 4401 means the login ended and the panel signs in again. The decisions are in `core/panel_feed.hpp`; `main/server_link.cpp` only opens the socket and passes on what happens to it.

**Acknowledging.** Anyone who can touch the panel can acknowledge, and acknowledging a critical alarm stops the server from going on warning people, so the panel asks first. `controls.ack` chooses what: `pin` (the default) opens the keypad of the disarm and sends the PIN of the panel's account with the order, `{"pin": "..."}`, which the server checks under the same lockout as a disarm (a wrong PIN is refused with 403 `wrong_pin` and nothing is acknowledged, too many answer 429 `pin_locked`, the right one counts as a person standing there and the escalation of a critical alarm stops, and the duress PIN acknowledges without that); `confirm` asks a yes or a no and sends `{}`, which the server takes as an unverified acknowledgement; `off` sends at once, as the panel always did. The rules are in `core/ack_flow.hpp` and the same function says what any answer to an order means (`order_error`), for this order and for arming and disarming; `tests/test_ack_flow.cpp` also compares the order with the OpenAPI of ARMOR-COMMON and the route of ARMOR-SERVER when they are next to this repository. The screen is the same question that arming and disarming use (`main/ui.cpp`); nothing of it has been seen on the real screen.

**The check before arming (`controls.preflight`, off by default).** When the person presses Arm, the panel asks the server for `GET /api/v1/preflight` and the confirmation shows what it found: the findings that block first (a node that does not answer, a silent alarm panel, an open zone, a door or window that is open, an alarm nobody has seen), then the warnings, five lines at most and "and more" after them. The button says "Arm anyway" while something is found. The check informs and never stops the person: if the server does not answer within eight seconds, refuses this account (the server's option `panel.preflight` is off) or sends something else, the confirmation is the old one and the panel arms exactly as before. The words are in the eight languages (`pf_*`).

**The countdown of the delays (`controls.countdown_beeps`, off by default for the beeps).** When the summary carries `delay` (the exit delay after arming, or the entry delay of a door that opened while armed), a bar over the screen says "Leave the house" or "Disarm now" with the seconds left, counted on the panel's own clock between two summaries; with the setting on it beeps once a second, twice a second in the last five. A summary without a countdown ends it at once.

**The camera picture (`camera.enabled`, off by default).** With `camera.id` set to the id of a camera in Studio, a Camera button opens its picture over the screen; while it is open the link asks the server for `GET /api/v1/cameras/{id}/snapshot` every `camera.refresh_s` seconds (2 to 60; a failure waits longer, up to a minute) and keeps only the latest, which is freed when it is closed. The panel accepts a baseline JPEG of at most 1 MiB and 640 x 480 pixels and says "No picture" otherwise. The server must allow it (its option `panel.camera_view`, off by default). The decoding uses the small JPEG decoder of LVGL (`CONFIG_LV_USE_TJPGD`); it has been tried on a computer only, not on the screen.

The panel never asks for the whole state: a 64 KB limit of its JSON reader would make a large installation impossible to read.

The state of the link is a small machine (`LinkTracker`): *not set up*, *no network*, *connecting*, *online*, *stale* (four polls without an answer: what is on the screen is old, and
the button to arm or disarm is disabled so that nobody acts on old news; the same goes when there is no network or the login was refused, which show what the panel knew, dimmed, and give no orders) and *denied* (the server refused the login twice in a row, or answered 403: someone has to fix the user in the
settings). After a failure the panel waits longer each time (up to a minute), so that a server that is down is not asked every three seconds.

The panel also publishes `armor/node/<id>/health` (with `"kind":"panel"`, so the server keeps it apart from the radars: it is listed in the services as a wall panel, it does not count as a radar node and its silence is not a node down) and, now and then, `armor/node/<id>/info` (its name, firmware and the address of its page) over MQTT, so that Studio lists it with a
link to its web page like any other node.

## The screen

800 by 480, drawn with LVGL 9: the mode of the system in the colour of the worst alarm nobody has seen, a large button to arm or disarm (with a question first), the number of nodes
online, the alarms (the unseen first, six at most) with a button to acknowledge them all, the microphone and a line at the bottom that says what is wrong (no server in the settings,
the server not answering, or how to set the panel up). The night hours dim it, a period without a touch puts it to sleep, and a touch or a new alarm wakes it; a new alarm also plays a
tone. The words are in eight languages (`core/screen_text.hpp`); Japanese and Chinese are drawn with fonts generated for exactly the characters the texts use (see [FONTS](FONTS.md)).

## What is not here

The wake word ("armor"), a view of the cameras on the screen, and a first power-up of the firmware. See
[VOICE](VOICE.md) and [HARDWARE](HARDWARE.md).
