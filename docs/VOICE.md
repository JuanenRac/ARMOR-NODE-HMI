# ARMOR-NODE-HMI - the voice assistant

The panel has four microphones and a speaker so that it can be the "home assistant with a big screen" of the installation, and so that the AI planned for the Jetson Orin NX can talk
to the people in the house through it. This version has the panel's half of that, in its simplest honest form.

## What there is

* **Push to talk.** A touch on the microphone button opens the microphone; a second touch (or the time limit, 8 seconds by default) closes it. The audio goes to the speech service
  whose address is in the settings, the reply is shown and spoken. Nothing is sent anywhere unless the voice is turned on in the settings and given an address.
* **A conversation as a state machine** (`core/voice_session.hpp`, tested in `tests/test_voice.cpp`): idle, listening, thinking, speaking, failed. Every step has a time limit
  (a reply that never ends does not lock the screen), a touch always stops it, a failure is shown for a few seconds and dismissed by a touch, and a sound right after a conversation
  (the speaker's own) is not taken for a new request.
* **The panel carries out no command by itself.** Whatever changes something (arming, a switch) is confirmed by the service with a question the person answers, as ARMOR-VOICE-AI's
  signed single-use confirmation does; the panel only shows and plays what the service says.

## What there is not

* **A wake word.** "Armor" cannot be said yet: a wake word needs a model for the chip (Espressif's ESP-SR) trained for the name, and the board's four microphones can feed it, but it is
  not part of this version. The setting "name it answers to" is what the screen will show once it exists.
* **A speech service that listens everywhere.** ARMOR-VOICE-AI answers `POST /v1/turn` (speech to text and text to speech by commands the installation chooses), but by default it listens only on the machine
  it runs on (port 18090): to reach it from the panel it must be started to accept other machines, or sit behind a proxy, and the panel must carry its token.

## The contract with the speech service

```
POST <voice.url>/v1/turn
Content-Type: audio/L16;rate=16000      16 kHz, mono, 16-bit signed little-endian PCM, at most listen_s seconds
X-Armor-Language: en|es|de|fr|it|ja|zh|pt the language of the panel
X-Armor-Voice-Token: <token>            the token of the service (the setting *Token*, never shown again once saved)

200 {"heard":"arm the system", "reply":"Arm the system? Say yes to confirm.", "audio":"<base64 of 16 kHz mono 16-bit PCM>"}
```

### The binary request

With `voice.binary_turn` on (the default) the panel sends the same turn as `multipart/form-data` (the *Voice turn* contract of ARMOR-COMMON): a `meta` part (`language`, `encoding` `pcm_s16le`, `sample_rate` 16000, `channels` 1, `reply_audio` `binary`) and an `audio` part with the recording as raw bytes, streamed from the recording without a second copy. The answer is `multipart/mixed`: a `result` part (the JSON without the audio, with `audio_omitted` `too_large` when the server left a long spoken answer out) and an `audio` part with the spoken answer as raw bytes, a third smaller than base64 and with nothing to decode on the chip. `core/voice_multipart.hpp` builds the framing and reads the answer as strictly as the reference reader, and is tested with the 43 shared cases.

A server that does not take it (the administrator turned it off with `voice.multipart_turn`, or the address is a service that never learned it) answers `415` (or `400` or `422`, which a speech service of another make answers to a body it does not know): the panel sends the same recording at once the older way, so the person does not have to speak twice, and does not try the binary form again for ten minutes. A server that answers the binary request with JSON is read as the older answer. With the setting off, only the older request is sent.

`heard` and `audio` are optional (without audio only the text is shown); a reply with neither text nor audio, a non-200 answer or a malformed body is a failure shown on the screen
("I could not do that"). The panel listens for at most 15 seconds (the service takes 480 KB of audio) and accepts at most 320 KB of answer (the service sends at most 200 KB of audio, which is about 267 KB as base64): the audio is
taken out of the text by hand (`core/voice_answer.hpp`) because the JSON reader of the node refuses documents over 64 KB, and an answer longer than 320 KB is refused whole and said so (*reply_too_long*).

## Privacy

The microphone is open only between the touch and the end of the recording; no audio is stored on the panel and none leaves it unless the voice is on. The address of the service is the
installation's own (a machine of the local network); an `https://` address is checked against the usual certificate authorities.

## Alarms said out loud and sounds of their own

Besides its tones, the panel can play two kinds of audio that ARMOR-SERVER makes, chosen for each origin of an alarm (`audio.tone_by_source.<origin>`: `spoken` or `custom`; the Sound card of the panel). Nothing is asked for until an origin is set to one of them, and the server has its own switch for each (the options `alarm_audio.spoken` and `alarm_audio.custom_sounds` in Studio, both off by default).

* `spoken`: the sentence of the alarm (how serious, where from and what it is, in the language of the panel) made by the voice gateway: `GET /api/v1/alarm-audio/spoken?language=&source=&severity=&code=`. The panel sends only the closed values and the alarm code if it is a plain one (lower-case letters, digits and `_.:-`); the server keeps the sentences it has made.
* `custom`: the short sound an administrator uploaded in Studio for that origin: `GET /api/v1/alarm-audio/sounds/<origin>`.

Both answers are raw 16 kHz 16-bit mono PCM with its SHA-256 in `X-Armor-Audio-Sha256`. Before playing, the panel checks (core/alarm_clip.hpp) that the answer is not empty, not shorter than a tenth of a second, not longer than `audio.clip_max_bytes` (default 200000 bytes, 3200 to 262144), a whole number of 16-bit samples, and that its SHA-256 is the one announced. Anything else is thrown away. It asks on a task of its own, one download at a time, and the alarm never waits for it: if the audio does not arrive whole (the option is off on the server, no sound was uploaded, the server is slow or unreachable, the checks fail), the alarm sounds as its severity says, so it is never silent for that reason. A siren for a critical alarm of another origin is never replaced by the audio of a lesser alarm. The panel remembers the last eight sentences (up to 600 KB) so that an alarm of the same kind is said at once; a sound of its own is asked for every time, so a new upload counts straight away. The download and the playing run only in the firmware and have not been tried on a board; the checks and the choice of what to play are tested on a computer.
