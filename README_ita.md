<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="Banner di ARMOR-NODE-HMI" width="100%">
</p>

# 🖥️ ARMOR-NODE-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  🇮🇹 <b>Italiano</b> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### Il pannello touch della casa: uno schermo da 7 pollici a parete che mostra lo stato del sistema, lo attiva e disattiva e riconosce gli allarmi, con microfono e altoparlante per l'assistente vocale che arriverà (un nodo della famiglia sull'ESP32-S3-Touch-LCD-7C-BOX, con la sua pagina web per le impostazioni)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 o successiva">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Linguaggio">
  <img src="https://img.shields.io/badge/Board-ESP32--S3--Touch--LCD--7C--BOX-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Checks-1%2C759-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-93081%20checks-2ea44f.svg" alt="Test">
  <img src="https://img.shields.io/badge/Version-0.4.0-0f6cbd.svg" alt="Versione">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturità">
</p>

---

**Controllo di onestà - cosa funziona oggi:** Ciò che non dipende dalla scheda è reale e provato su un computer (le impostazioni, ciò che il pannello sa del server e quando è obsoleto, l'allarme di coercizione che non viene mai mostrato, la conversazione vocale e la lunga risposta del servizio vocale, i pin della scheda, i testi dello schermo in otto lingue e i glifi dei suoi caratteri, i controlli di versione e firma: 30,783 verifiche in 18 programmi di test) e lo è anche la pagina web (eseguita contro un sostituto in un browser reale). **Il firmware è stato compilato una volta (ESP-IDF 5.5.5, non verificato dall'integrazione continua) e non è mai girato su una scheda:** lo schermo, il touch, l'audio e il collegamento al server sono scritti dagli esempi di Waveshare per questa famiglia di schede e attendono la prima accensione. Il servizio vocale (ARMOR-VOICE-AI) risponde alle richieste di questo pannello; non c'è una parola di attivazione: si tocca per parlare.

---

## 📋 In breve

| | |
|---|---|
| Versione | `0.4.0` |
| Maturità | Funzionale |
| Ruolo | Pannello touch a parete per ARMOR-SERVER (famiglia: Clients, padre: ARMOR-SERVER) |
| Tecnologia | C++17, ESP-IDF 5.5.5, LVGL, ESP32-S3 |
| Test | 93081 verifiche in 18 programmi host e 6 verifiche Python (`ctest`) |
| Scheda | Waveshare ESP32-S3-Touch-LCD-7C-BOX (800x480, GT911) |
| Gira su | Una Waveshare ESP32-S3-Touch-LCD-7C-BOX; test su host su qualsiasi computer |
| Su hardware reale | Ancora nessuno: il firmware si compila (ESP-IDF 5.5.5, non verificato dalla CI) ma non è girato sulla scheda |
| Licenza | GPL-3.0-or-later |

**Vai a:** [Panoramica](#-panoramica) · [Architettura](#-architettura) · [Sicurezza](#-sicurezza-e-protezione) · [Interfacce](#-interfacce) · [Configurazione](#-configurazione) · [Avvio rapido](#-avvio-rapido) · [Test](#-test) · [Problemi](#-risoluzione-dei-problemi) · [Struttura](#-struttura-del-repository) · [Correlati](#-progetti-correlati)

## 🚦 Stato del progetto

Ogni riga è una parte di questo repository e il semaforo dice a che punto è oggi. È scritta in base a ciò che il repository può mostrare (test, compilazioni, esecuzioni), non a ciò che è previsto.

🟢 Fatto e verificato · 🟡 Funziona, con limiti noti · 🟠 Fatto a metà o non ancora verificato · 🔴 Non fatto o non funzionante

| Area | Stato | Cosa significa qui |
|---|---|---|
| Software | 🟠 | Il firmware è stato compilato una volta (ESP-IDF 5.5.5), ma la compilazione non è controllata dall'integrazione continua e non ha mai girato su una scheda. |
| Test | 🟢 | Superano 93.081 controlli in 18 programmi host (`ctest`): le impostazioni, ciò che il pannello sa del server e quando è obsoleto, l'allarme sotto minaccia che non viene mai mostrato, lo scambio vocale, i pin, i testi dello schermo in otto lingue e i glifi dei suoi caratteri. |
| Protocolli e dati | 🟡 | Lo scambio con il server e con il servizio vocale ARMOR-VOICE-AI è provato contro simulazioni; è push to talk, senza parola di attivazione. |
| Interfaccia | 🟠 | La pagina web è stata eseguita contro una simulazione in un browser reale; lo schermo, il tocco e l'audio sono scritti dagli esempi di Waveshare e attendono la prima accensione. |
| Hardware | 🔴 | Il firmware non ha mai girato sulla scheda: nulla ha ancora funzionato su apparecchiature reali. |
| Sicurezza | 🟡 | Il codice di configurazione, il limite di accessi, il PIN Bluetooth, la gestione della minaccia e gli aggiornamenti firmati sono testati sull'host; non c'è stata una revisione indipendente. |
| Aggiornamenti | 🟠 | I controlli di hash e firma passano sull'host; un aggiornamento via etere non è mai stato eseguito su una scheda. |
| Integrazione | 🟡 | I messaggi sono validati contro gli schemi condivisi di ARMOR-COMMON; il collegamento con il server e con il servizio vocale non è stato provato da una scheda. |
| Documentazione | 🟢 | Otto lingue, la guida del firmware, le note di cablaggio e il registro delle modifiche sono aggiornati. |

## 🎯 Panoramica

* **Un client di ARMOR-SERVER, come l'app Android:** accede con un utente creato per lui da un amministratore, chiede `GET /api/v1/panel/summary` (poche centinaia di byte: la modalità, i nodi online e gli allarmi che richiedono una persona) ogni pochi secondi e, quando la persona tocca il pulsante e conferma, attiva, disattiva o riconosce. Se il server smette di rispondere, lo schermo dice che ciò che mostra è vecchio e il pulsante è disattivato. Ascolta il canale in diretta del server (un WebSocket) così un cambiamento si vede subito, e interroga come prima quando il canale manca, tace o è disattivato.
* **Lo schermo (800x480, LVGL):** la modalità nel colore del peggior allarme che nessuno ha visto, un grande pulsante attiva/disattiva con domanda prima, i nodi online, gli allarmi (quelli non visti per primi), una notte che lo attenua, un riposo dopo un tempo senza tocchi, un suono e un risveglio all'arrivo di un nuovo allarme, e otto lingue con accenti, giapponese e cinese disegnati con font generati per esattamente i caratteri usati.
* **Un nodo come gli altri:** la stessa memoria delle impostazioni, Wi-Fi, pagina web in otto lingue, configurazione Bluetooth dal telefono, aggiornamento del firmware con ripristino e HTTPS come ARMOR-NODE-RADAR, ARMOR-NODE-SOLAR e ARMOR-NODE-ELECTRICAL (condividono `ARMOR-COMMON/firmware_base`), più pagine per server, schermo, suono e voce; segnala a Studio di esistere via MQTT perché Studio lo elenchi con la sua pagina.
* **L'assistente vocale, premi per parlare:** un tocco apre il microfono, l'audio va solo al servizio vocale il cui indirizzo è nelle impostazioni (disattivato per impostazione predefinita), la risposta viene mostrata e detta; l'ordine dei passi, i limiti di tempo e ciò che fa un tocco sono una macchina a stati provata, e il pannello non esegue alcun comando da solo ([il contratto](docs/VOICE.md)). La registrazione parte come byte in una richiesta binaria e la risposta parlata torna come byte; a un server che non la accetta si invia la richiesta precedente.
* **La scheda:** Waveshare ESP32-S3-Touch-LCD-7C-BOX (32 MB di flash, 16 MB di PSRAM, schermo RGB da 7 pollici con tocco GT911, quattro microfoni, un altoparlante); i pin vengono dagli esempi di Waveshare e un test dimostra che due funzioni non condividono un GPIO ([la scheda](docs/HARDWARE.md)).
* **Aggiornamenti via etere firmati:** il nodo installa da GitHub solo un'immagine il cui SHA-256 coincide e che porta una firma Ed25519 valida del responsabile (la chiave pubblica è nel firmware, quella privata non lascia mai il responsabile); un file caricato a mano da un amministratore nel pannello resta accettato. Le pagine e i messaggi del pannello sono in otto lingue.
* **Abbinamento Bluetooth con PIN:** il PIN a sei cifre deriva dal codice di configurazione, è stampato sulla console USB e mostrato a un amministratore nel pannello; il telefono deve inserirlo prima di poter scrivere le impostazioni del nodo.
* **Non ancora:** una prima compilazione e accensione del firmware, una parola di attivazione, il servizio vocale sul Jetson e una vista delle telecamere sullo schermo.
* **Solo la casa può usare il pannello:** il pannello risponde per impostazione predefinita alla rete privata (o alle reti indicate dal proprietario, come una VPN), rifiuta un elenco che escluderebbe chi lo salva, conta le password errate per persona e per indirizzo e funziona dietro il proxy del server per essere raggiunto da fuori casa senza pubblicare il nodo.
* **Dice cosa succede:** lo schermo conosce il testo, in otto lingue, di ogni codice d'allarme che il server solleva (rete, solare, elettrico, zone d'allarme...) e dice al server di essere un pannello a parete, quindi viene elencato come tale e non conta come radar.
* **Collegamento cifrato facoltativo con il broker (TLS):** disattivato per impostazione predefinita, quindi il nodo parla MQTT non cifrato come prima. Un interruttore e una casella per il certificato dell'autorità propria dell'installazione (PEM, al massimo 2 KB) nella pagina Broker lo fanno collegare con `mqtts://`, fidarsi solo di quell'autorità e verificare che il certificato del broker sia stato fatto per l'indirizzo raggiunto; ciò che non è un solo certificato PEM viene rifiutato. Coperto da test sul computer; l'handshake non è ancora stato eseguito su una scheda.
* **Rapporto di salute:** circa una volta al minuto il nodo pubblica da quanto è acceso e perché si è riavviato l'ultima volta, la memoria libera, il collegamento e il segnale Wi-Fi, quante volte si è connesso al broker e l'ultimo errore registrato (`armor/health/<nodo>`, il contratto `node_health` di ARMOR-COMMON). Il codice che lo costruisce è condiviso da tutti i nodi e testato su un computer; non è mai girato su una scheda.
* **Controllo dal server:** Studio può riavviare il nodo in modo pulito, metterlo in pausa fino a un'ora (riprende da solo), mettere il suo lavoro in standby (senza fine, conservato ai riavvii, il nodo resta connesso e continua a rispondere), farlo mostrare, chiedere un'istantanea diagnostica e segnarlo in manutenzione (`armor/control/<nodo>/...`, il contratto `node_control` di ARMOR-COMMON). Ogni ordine è in due passi con un token monouso legato a quell'operazione, solo il server può scriverli sul broker e non esistono ripristino di fabbrica, cancellazione né sonno profondo. Finché il pannello è in pausa o in standby lo schermo mostra una schermata propria che lo dice e non accetta tocchi; quando gli si chiede di identificarsi, in alto lampeggia una barra (mai un suono). Il codice è condiviso da tutti i nodi e provato su un computer; non ha ancora girato su una scheda.
* **Diagnostica senza cavo:** dopo un avvio non pulito (un blocco, un watchdog, un calo di alimentazione, un'allocazione fallita) il rapporto di salute ne indica la causa per dieci minuti; un arresto anomalo lascia il suo core dump da vedere, scaricare o cancellare dal pannello; le impostazioni salvate vengono copiate prima di ogni installazione del firmware e recuperate se un aggiornamento le danneggia; e un amministratore può chiedere al nodo, tramite il server, un pacchetto di diagnostica o il suo registro con un token monouso.
* **Una sirena che si ripete e suoni in ordine:** ogni suono passa per una coda con priorità (sirena, allarme, voce, conferma, avviso); la sirena si ripete finché restano allarmi critici non riconosciuti (`audio.siren_repeat_s` 30, `audio.siren_max_repeats` 10) e gli allarmi mantengono un volume minimo (`audio.critical_min_volume` 40); la voce cede a un allarme.
* **Un orologio con batteria:** il chip PCF85063A della scheda dà l'ora all'avvio quando il nodo non ha ancora un'ora reale e il chip è affidabile, e viene corretto a ogni sincronizzazione o impostazione dell'ora (`time.rtc_enabled`, attivo per impostazione predefinita).
* **Lo stato di un comando, sullo schermo:** il piè di pagina dice che il comando è in invio finché è in viaggio e che è stato confermato per qualche secondo quando è andato a buon fine; un rifiuto vi mantiene il suo motivo, come prima.
* **Sigillatura facoltativa dei segreti salvati:** disattivata per impostazione predefinita (`security.seal_secrets`, o `nodes.nvs_encryption` del server). Attiva, le password, i token, le chiavi, i PIN e gli hash delle password che il nodo salva restano sigillati con ChaCha20-Poly1305 e tutto il resto resta leggibile; attivarla o disattivarla li riscrive senza perdere nulla, anche se il nodo si ferma a metà. Nessun eFuse viene bruciato e la flash non è cifrata: la chiave si trova in una partizione dello stesso chip, quindi **non** protegge da chi ha la scheda, e un nodo installato prima di quella partizione mantiene i suoi segreti com'erano finché non viene flashato con la nuova tabella delle partizioni.

## 🧩 Ruolo in A.R.M.O.R.

ARMOR-NODE-HMI è il pannello a muro: un client di [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), come l'app Android, con uno schermo da 7 pollici, un microfono e un altoparlante. Mostra la modalità, i nodi online e gli allarmi che richiedono una persona, inserisce e disinserisce (con il PIN) e riconosce gli allarmi. È dove vivrà l'assistente vocale ([ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)) quando esisterà il suo servizio vocale e, come ogni nodo, si configura e si aggiorna da [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO).

| Nome | Come | A cosa serve |
|---|---|---|
| ARMOR-SERVER | HTTPS `GET /api/v1/panel/summary`, WebSocket `/api/v1/panel/events` | Il riepilogo della casa (modalità, allarmi, nodi, energia) e le sue variazioni; il server decide cosa fa un ordine |
| ARMOR-VOICE-AI | tramite il server | L'audio di un turno va al servizio vocale; la risposta è mostrata e pronunciata |
| ARMOR-STUDIO | menu Pannello touch | Mostra la salute del pannello e apre il suo pannello web |
| ARMOR-COMMON | base firmware | Il firmware comune (pannello, accessi, aggiornamento firmato, configurazione Bluetooth) è copiato da lì |

## 🏛️ Architettura

<p align="center">
  <img src="images/architecture.svg" alt="Architettura di ARMOR-NODE-HMI: chi lo chiama, cosa contiene e da cosa dipende" width="100%">
</p>

## 🖼️ Schermate

Catturate a 1920×1080 su un banco di prova, con dati di esempio e senza alcuna installazione reale dietro.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Riepilogo: lo stato del nodo in breve - rete, broker, firmware e ciò che riporta." width="100%"><br><sub>Riepilogo: lo stato del nodo in breve - rete, broker, firmware e ciò che riporta.</sub></td><td width="50%" valign="top"><img src="images/screenshots/screen.png" alt="Schermo e suono: luminosità, ore notturne, volume e quali allarmi vengono annunciati." width="100%"><br><sub>Schermo e suono: luminosità, ore notturne, volume e quali allarmi vengono annunciati.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/voice.png" alt="Assistente vocale: premi per parlare, disattivato per impostazione predefinita, e il servizio a cui va il suo audio." width="100%"><br><sub>Assistente vocale: premi per parlare, disattivato per impostazione predefinita, e il servizio a cui va il suo audio.</sub></td><td width="50%" valign="top"><img src="images/screenshots/server.png" alt="Server: l'indirizzo del server A.R.M.O.R., l'utente del pannello e ogni quanto interroga." width="100%"><br><sub>Server: l'indirizzo del server A.R.M.O.R., l'utente del pannello e ogni quanto interroga.</sub></td></tr>
</table>

## 🔒 Sicurezza e protezione

| Minaccia | Difesa |
|---|---|
| Password in chiaro o compilate | PBKDF2-HMAC-SHA256 con 10 000 round; nessuna password nel firmware (un nodo senza utenti entra in configurazione); al massimo 4 utenti (admin o visualizzatore) e da 8 a 64 caratteri; al massimo 6 sessioni, 30 minuti di inattività |
| Prendere un nodo appena programmato | La configurazione richiede un codice di avvio: quello della build (`CONFIG_ARMOR_SETUP_CODE`) o uno casuale tenuto in flash e mostrato sulla console USB; con un segreto di flotta è `HMAC-SHA256(segreto, MAC)`; è anche la chiave del Wi-Fi di configurazione (WPA2, mai in bridge) |
| Il pannello raggiunto da Internet | Risponde di default alle reti private di una casa (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 e 100.64/10 dove vivono alcune VPN); l'impostazione `web.admin_sources` elenca altri indirizzi e reti IPv4 (8 voci, 120 caratteri); tutto il resto è rifiutato tranne l'identità pubblica `GET /api/v1/session`; una lista che escluderebbe chi la salva è rifiutata |
| Il pannello in chiaro su HTTP | HTTPS sulla 443 con un certificato che il nodo crea da sé (autofirmato, in flash, impronta SHA-256 visibile per confrontarla); il pannello può essere http, https o entrambi; il cookie di sessione è sempre HttpOnly e SameSite=Strict (e Secure su TLS) |
| Un firmware falso da GitHub | Un'immagine si installa solo con uno SHA-256 coincidente e una firma Ed25519 valida fatta con la chiave del manutentore (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` la rifiutano); un caricamento manuale da un amministratore non è verificato: un limite esplicito |
| Dirottamento della configurazione Bluetooth | Un PIN a 6 cifre derivato dal codice di configurazione, visibile sulla console USB e a un amministratore; il telefono deve digitarlo prima di scrivere le impostazioni; la finestra dura `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 di default, da 1 a 120) |
| Segreti memorizzati letti da chi ha la scheda | Opzionale (spento di default, interruttore `security.seal_secrets` o `nodes.nvs_encryption` del server): ChaCha20-Poly1305 per password, token, chiavi, PIN e hash. Non cifra la flash né brucia un eFuse: la chiave è in una partizione dello stesso chip, quindi non protegge da chi ha la scheda |
| Il collegamento al broker in chiaro | TLS opzionale (`mqtts://`) con l'autorità propria del proprietario (PEM fino a 2 KB) e verifica del nome; spento di default. L'handshake è girato solo nei test su host, non su una scheda |
| L'allarme sotto coercizione mostrato sullo schermo | Non viene mai mostrato (un test su host lo verifica); e ciò che il pannello sa del server è segnato come scaduto quando è vecchio |
| Un server che non è il server | Il certificato del server è fissato (`server_pin`), per le richieste e per il WebSocket del canale live; il pannello accede con un proprio utente, creato da un amministratore, con il ruolo `panel` (non può usare Studio) |
| Disinserire senza PIN | Inserire e disinserire chiedono prima e prendono il PIN sullo schermo (`pin_entry`); il server lo ricontrolla |
| Audio vocale verso un posto inatteso | Spenta di default (`voice.enabled` falso); l'audio va solo al servizio il cui indirizzo è nelle impostazioni (l'indirizzo è validato e cambiare host cancella il token); il pannello non esegue alcun ordine da solo e non c'è parola di attivazione: è premi per parlare |
| Una sirena che nessuno sente | Una sola coda di priorità (sirena, allarme, voce, conferma, avviso); la sirena si ripete finché ci sono allarmi critici non confermati (`audio.siren_repeat_s` 30, da 10 a 600, 0 = nessuna ripetizione; `audio.siren_max_repeats` 10) e il volume critico non scende mai sotto `audio.critical_min_volume` (40 %) |

Il firmware è stato compilato una volta e non è mai girato su una scheda: schermo, touch, suono e collegamento al server sono scritti dagli esempi di Waveshare, in attesa della prima accensione. Leggi [SECURITY.md](SECURITY.md) per segnalare una vulnerabilità.

## 🔌 Interfacce

| Parte | Cosa imposta |
|---|---|
| ARMOR-SERVER | `GET /api/v1/panel/summary` ogni `server.poll_s` (3 s, da 1 a 60) solo mentre il canale live è caduto, e il WebSocket `/api/v1/panel/events` finché `server.live_events` è vero (predefinito): il riepilogo arriva quando cambiano modalità, allarmi, nodi o energia; l'utente proprio del pannello ha il ruolo `panel` |
| Voce | L'audio di un turno va a ARMOR-VOICE-AI tramite il server (`/api/v1/voice/turn`, con corpo multipart facoltativo); `core/voice_session.hpp` è la macchina a stati (ordine dei passi, limiti di tempo, cosa fa un tocco); vedi [docs/VOICE.md](docs/VOICE.md) |
| Hardware | Waveshare ESP32-S3-Touch-LCD-7C-BOX: schermo RGB da 7 pollici 800x480, touch GT911, quattro microfoni, un altoparlante, RTC PCF85063A (`time.rtc_enabled`, vero di default), 32 MB di flash e 16 MB di PSRAM; un test dimostra che nessuna funzione condivide un GPIO |

## ⚙️ Configurazione

| Variabile | Predefinito | Cosa imposta |
|---|---|---|
| `server.poll_s`, `server.live_events` | 3 (1-60), true | Il periodo di interrogazione del riepilogo e il canale live via WebSocket |
| `display.sleep_s` | 300 (0-3600) | Secondi prima che lo schermo vada a riposo (0 = mai); anche la luminosità e la luminosità notturna sono impostazioni |
| `audio.critical_min_volume`, `audio.siren_repeat_s`, `audio.siren_max_repeats` | 40 (0-100), 30, 10 | Il volume minimo di un suono critico, il periodo di ripetizione della sirena (0 o da 10 a 600 s) e le sue ripetizioni massime |
| `voice.enabled`, `voice.url`, `voice.token` | false, "", - | Il servizio vocale: spento di default; l'indirizzo è lungo al massimo 160 caratteri e viene validato (altrimenti `required` o `invalid`); il token si cancella quando cambia l'host |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | Le impostazioni che tutti i nodi condividono: il nome (vuoto dà `armor-` e gli ultimi byte del MAC), il codice di configurazione (vuoto: casuale), il segreto di flotta (16+ caratteri, mai nel repository), le prime impostazioni del broker, il battito (da 2 a 300 s), i minuti di configurazione Bluetooth (da 1 a 120) e il ripristino con BOOT (8 s nei primi 30 s) |

## 🚀 Avvio rapido

**Requisiti:** un compilatore C++17 per i test su host; il container ESP-IDF 5.5.5 per costruire l'immagine; una Waveshare ESP32-S3-Touch-LCD-7C-BOX.

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel of the node without a board
tools/build_node.sh                                   # the image in the ESP-IDF container
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

Vedi la [progettazione](docs/DESIGN.md), la [scheda](docs/HARDWARE.md), l'[assistente vocale](docs/VOICE.md), la [guida al firmware](docs/NODE_FIRMWARE.md) e i [font](docs/FONTS.md).

Per compilare il firmware a mano in GitHub Actions, vedi [compilare in GitHub Actions](docs/FIRMWARE_BUILD_ACTIONS.md).

## 🧪 Test

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

18 programmi di test su host e 6 verifiche Python girano con `ctest`; 93081 verifiche contate dai programmi host, compilati con `-Werror`.

Verifiche Python: i testi del pannello in otto lingue, i task, i percorsi di salvataggio, la sessione Bluetooth sotto il suo lock e i percorsi chiamati dal pannello (e, in alcuni nodi, i codici HTTP o i glifi dei caratteri)

I test su host richiedono un qualsiasi compilatore C++17 (Linux, WSL, MSYS2); il pannello web è anche provato in un browser reale contro un nodo sostitutivo (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

`check_font_glyphs.py` verifica che i caratteri abbiano tutti i glifi delle otto lingue (98 righe); `test_clock` stampa solo `ok`.

## 🏗️ Compilazione e rilascio

`tools/build_node.sh` costruisce l'immagine nel container ESP-IDF 5.5.5 e la versione porta l'immagine, il suo SHA-256 e la sua firma Ed25519, una per scheda; l'immagine si scrive via USB-C la prima volta e poi si aggiorna dal pannello.

## 🩺 Risoluzione dei problemi

| Vedi | Perché | Fai |
|---|---|---|
| Hai perso la password o il nodo non entra più in rete | Un nodo in configurazione senza indirizzo dopo 90 s apre un proprio Wi-Fi `ARMOR-SETUP-xxxxxx` (apri `http://192.168.4.1/`) | Per ricominciare tieni premuto BOOT per 8 secondi nei primi 30 secondi dopo l'accensione (cancella impostazioni e utenti), o usa *Ripristino di fabbrica* nel pannello |
| Un aggiornamento da GitHub è rifiutato | La versione non ha checksum o firma, non corrispondono (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), non ha un'immagine per questa scheda (`e_no_asset_for_board`) o è di un altro progetto (`e_wrong_firmware`) | Il nodo non è cambiato; riprova o lascia che un amministratore carichi il file a mano |
| La voce non parte | `voice.enabled` è spento, o `voice.url` è `required` o `invalid` | Accendila nel pannello e dai un indirizzo valido; cambiare host cancella il token |
| Il piè di pagina dice invio, poi confermata o il motivo | Un ordine è in viaggio, è stato confermato per qualche secondo, o è stato rifiutato e conserva il suo motivo | Leggi il motivo; decide il server, non il pannello |

## 📂 Struttura del repository

<p align="center">
  <img src="images/repository-structure.svg" alt="Struttura del repository di ARMOR-NODE-HMI" width="100%">
</p>

<details>
<summary>Versione testuale dell'albero</summary>

```text
ARMOR-NODE-HMI/
├── main/                                il firmware: un componente ESP-IDF
├── core/                                la logica senza ESP-IDF, testata su un computer
├── panel/                               il pannello web incorporato nel firmware
├── tests/                               test su host e verifiche Python
├── docs/                                9 guide: progetto, voce, caratteri, hardware
├── tools/                               strumenti di build, impacchettamento e verifica
├── .github/                             workflow CI e modelli per issue e pull request
├── images/                              banner, icona e le due immagini di questo README
├── build.sh build.bat                   build tramite il flusso di progetto condiviso
├── test.sh test.bat                     eseguire i test tramite il flusso condiviso
├── run.sh run.bat                       avvio tramite il flusso di progetto condiviso
├── armor.project.json                   manifesto: versione, ruolo, maturità, comando di build
├── CHANGELOG.md                         cosa è cambiato in ogni versione
├── SECURITY.md SUPPORT.md LICENSE       politica di sicurezza, supporto e licenza
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   come contribuire e il codice di condotta
├── CMakeLists.txt                       il progetto ESP-IDF
├── partitions.csv                       la tabella della flash
├── sdkconfig.defaults                   le impostazioni del firmware
├── sdkconfig.board.lcd7box              le impostazioni di una scheda
├── secrets/                             esempio del file del nodo (quelli veri non sono versionati)
├── dependencies.lock                    le versioni fissate dei componenti ESP-IDF
├── build-test.sh build-test.bat         eseguire i test tramite il flusso condiviso
└── .gitattributes .gitignore            impostazioni di Git
```

</details>

## 🗺️ Roadmap e limiti noti

**Limiti noti oggi**

* Il firmware è stato compilato una volta (ESP-IDF 5.5.5), ma la compilazione non è controllata dall'integrazione continua e non ha mai girato su una scheda.
* Lo scambio con il server e con il servizio vocale ARMOR-VOICE-AI è provato contro simulazioni; è push to talk, senza parola di attivazione.
* La pagina web è stata eseguita contro una simulazione in un browser reale; lo schermo, il tocco e l'audio sono scritti dagli esempi di Waveshare e attendono la prima accensione.
* Il firmware non ha mai girato sulla scheda: nulla ha ancora funzionato su apparecchiature reali.
* Il codice di configurazione, il limite di accessi, il PIN Bluetooth, la gestione della minaccia e gli aggiornamenti firmati sono testati sull'host; non c'è stata una revisione indipendente.
* I controlli di hash e firma passano sull'host; un aggiornamento via etere non è mai stato eseguito su una scheda.
* I messaggi sono validati contro gli schemi condivisi di ARMOR-COMMON; il collegamento con il server e con il servizio vocale non è stato provato da una scheda.

**Prossimi passi**

* Fare la prima compilazione completa e la prima accensione sulla scheda, poi il servizio vocale sul Jetson, una parola di attivazione e la vista delle telecamere sullo schermo.

## 🌍 Lingue

Il README e i testi esistono in inglese, spagnolo, francese, italiano, tedesco, cinese, giapponese e portoghese europeo.

## 🔗 Progetti correlati

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) è un sistema di sicurezza perimetrale fatto di repository indipendenti. Ognuno ha la propria versione, i propri test e il proprio README; ecco la famiglia:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contratti dei messaggi, validatori, vettori di conformità e tipi generati
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware del nodo di campo per ESP32-S3 con tre radar e un proprio pannello web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocolli di inverter e batterie solari e messaggi di un nodo gateway
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Nodo elettrico: contatori, il messaggio delle letture della rete e le regole di manovra
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nodo e centrale d'allarme: zone, inserimento, ritardi, sirena e PIN, con il server o senza
* **ARMOR-NODE-HMI** (questo repository) - Pannello touch: lo stato del sistema su uno schermo a parete, attivare e riconoscere gli allarmi, e la casa dell'assistente vocale
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - La rete locale: i suoi dispositivi, internet e ciò che cambia
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordinatore centrale: telemetria, allarmi, dispositivi, letture solari e telecamere
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Console web: telecamere, radar, allarmi, energia solare e progettista del sito 2D/3D
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Client Android dell'operatore con radar 2D/3D in tempo reale
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Client operatore per iPhone
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Politica di inferenza visiva che spiega le sue decisioni e non agisce mai
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zone delle telecamere, maschere e regole di rilevamento per la Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intenti vocali offline con una conferma impossibile da falsificare
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Contenitori, elettronica e matrice di accettazione da banco
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Distribuzione, banco di prova CM5, backup e TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulatore di telemetria offline con guasti ripetibili
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Rileva, installa e aggiorna i repository stessi dell'ecosistema
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architettura, base di sicurezza e matrice delle capacità

## 📚 Documentazione

* [Progetto](docs/DESIGN.md) e [hardware](docs/HARDWARE.md): cosa mostra il pannello e la scheda
* [Voce](docs/VOICE.md) e [caratteri](docs/FONTS.md)
* [Firmware del nodo](docs/NODE_FIRMWARE.md) e [azioni di build](docs/FIRMWARE_BUILD_ACTIONS.md)
* [Prove al banco](docs/BENCH_TESTS.md), [codice di configurazione](docs/SETUP_CODE.md) e [provisioning Bluetooth](docs/BLE_PROVISIONING.md)
* [Matrice delle capacità: cosa è provato e cosa no](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catalogo dei progetti: versioni e dipendenze tra i repository](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Cronologia delle modifiche di questo repository](CHANGELOG.md)

## 🤝 Contribuire e comunità

* Prima di una pull request esegui `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py`: deve passare. Vedi [CONTRIBUTING.md](CONTRIBUTING.md) e il [codice di condotta](CODE_OF_CONDUCT.md).
* Il README esiste in otto lingue ed è generato dalle sorgenti in ARMOR-DOCS: una modifica al testo si fa lì.
* Una modifica a un messaggio tra componenti parte da ARMOR-COMMON.
* Non descrivere come funzionante su una scheda ciò che è stato solo compilato.
* Segnala una vulnerabilità in privato: [SECURITY.md](SECURITY.md). Domande, idee e segnalazioni: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTORE

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENZA

GPL-3.0-or-later - vedi [LICENSE](LICENSE).
