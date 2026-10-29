<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="Bannière d'ARMOR-NODE-HMI" width="100%">
</p>

# 🖥️ ARMOR-NODE-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  🇫🇷 <b>Français</b> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### Le panneau tactile de la maison : un écran de 7 pouces au mur qui montre l'état du système, l'arme et le désarme et acquitte les alarmes, avec un micro et un haut-parleur pour l'assistant vocal à venir (un nœud de la famille sur l'ESP32-S3-Touch-LCD-7C-BOX, avec sa propre page web de réglages)

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 ou ultérieure">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Langage">
  <img src="https://img.shields.io/badge/Board-ESP32--S3--Touch--LCD--7C--BOX-e7352c.svg" alt="Board">
  <img src="https://img.shields.io/badge/Checks-1%2C759-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-93122%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.4.0-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturité">
</p>

---

**Vérification d'honnêteté - ce qui fonctionne aujourd'hui :** Ce qui ne dépend pas de la carte est réel et testé sur un ordinateur (les réglages, ce que le panneau sait du serveur et quand cela est périmé, l'alarme de contrainte qui n'est jamais affichée, la conversation vocale et la longue réponse du service vocal, les broches de la carte, les textes de l'écran en huit langues et les glyphes de ses polices, les contrôles de version et de signature : 30,783 vérifications en 19 programmes de test), de même que la page web (exécutée contre un substitut dans un vrai navigateur). **Le firmware a été compilé une fois (ESP-IDF 5.5.5, non vérifié par l'intégration continue) et n'a jamais tourné sur une carte :** l'écran, le tactile, le son et la liaison avec le serveur sont écrits d'après les exemples de Waveshare pour cette famille de cartes et attendent leur première mise sous tension. Le service vocal (ARMOR-VOICE-AI) répond aux requêtes de ce panneau ; il n'y a pas de mot d'activation : c'est appuyer pour parler.

---

## 📋 En un coup d'œil

| | |
|---|---|
| Version | `0.4.0` |
| Maturité | Fonctionnel |
| Rôle | Panneau tactile mural pour ARMOR-SERVER (famille : Clients, parent : ARMOR-SERVER) |
| Technologie | C++17, ESP-IDF 5.5.5, LVGL, ESP32-S3 |
| Tests | 93122 vérifications dans 19 programmes hôte et 6 contrôles Python (`ctest`) |
| Carte | Waveshare ESP32-S3-Touch-LCD-7C-BOX (800x480, GT911) |
| Fonctionne sur | Une Waveshare ESP32-S3-Touch-LCD-7C-BOX ; tests sur hôte sur n'importe quel ordinateur |
| Sur matériel réel | Aucun pour l'instant : le firmware se compile (ESP-IDF 5.5.5, non vérifié par la CI) mais n'a pas tourné sur la carte |
| Licence | GPL-3.0-or-later |

**Aller à :** [Présentation](#-présentation) · [Architecture](#-architecture) · [Sécurité](#-sécurité-et-protection) · [Interfaces](#-interfaces) · [Configuration](#-configuration) · [Démarrage rapide](#-démarrage-rapide) · [Tests](#-tests) · [Dépannage](#-dépannage) · [Structure](#-structure-du-dépôt) · [Liés](#-projets-liés)

## 🚦 État du projet

Chaque ligne est une partie de ce dépôt et le feu dit où elle en est aujourd'hui. Elle est écrite d'après ce que le dépôt peut montrer (tests, compilations, exécutions), pas d'après ce qui est prévu.

🟢 Fait et vérifié · 🟡 Fonctionne, avec des limites connues · 🟠 À moitié fait ou pas encore vérifié · 🔴 Pas fait ou ne fonctionne pas

| Domaine | État | Ce que cela veut dire ici |
|---|---|---|
| Logiciel | 🟠 | Le firmware a été compilé une fois (ESP-IDF 5.5.5), mais la compilation n'est pas contrôlée par l'intégration continue et il n'a jamais tourné sur une carte. |
| Tests | 🟢 | 93 122 vérifications dans 19 programmes d'hôte réussissent (`ctest`) : les réglages, ce que le panneau sait du serveur et quand c'est périmé, l'alarme sous contrainte qui n'est jamais affichée, l'échange vocal, les broches, les textes de l'écran en huit langues et les glyphes de ses polices. |
| Protocoles et données | 🟡 | L'échange avec le serveur et avec le service vocal ARMOR-VOICE-AI est testé contre des simulations ; c'est du push-to-talk, sans mot d'activation. |
| Interface | 🟠 | La page web a tourné contre une simulation dans un vrai navigateur ; l'écran, le tactile et le son sont écrits d'après les exemples de Waveshare et attendent leur première mise sous tension. |
| Matériel | 🔴 | Le firmware n'a pas tourné sur la carte : rien n'a encore tourné sur de vrais équipements. |
| Sécurité | 🟡 | Le code de configuration, la limite de connexions, le PIN Bluetooth, la gestion de la contrainte et les mises à jour signées sont testés sur l'hôte ; il n'y a pas eu de revue indépendante. |
| Mises à jour | 🟠 | Les contrôles de hash et de signature réussissent sur l'hôte ; une mise à jour par voie hertzienne n'a jamais été exécutée sur une carte. |
| Intégration | 🟡 | Les messages sont validés par rapport aux schémas partagés d'ARMOR-COMMON ; la liaison avec le serveur et avec le service vocal n'a pas été essayée depuis une carte. |
| Documentation | 🟢 | Huit langues, le guide du firmware, les notes de câblage et le journal des modifications sont à jour. |

## 🎯 Présentation

* **Un client d'ARMOR-SERVER, comme l'application Android :** il se connecte avec un utilisateur qu'un administrateur a créé pour lui, demande `GET /api/v1/panel/summary` (quelques centaines d'octets : le mode, les nœuds en ligne et les alarmes qui demandent une personne) toutes les quelques secondes et, quand la personne touche le bouton et confirme, arme, désarme ou acquitte. Si le serveur ne répond plus, l'écran dit que ce qu'il montre est ancien et le bouton est désactivé. Il écoute le canal en direct du serveur (un WebSocket) pour qu'un changement s'affiche aussitôt, et interroge comme avant quand le canal est absent, silencieux ou désactivé.
* **L'écran (800x480, LVGL) :** le mode dans la couleur de la pire alarme que personne n'a vue, un grand bouton armer/désarmer avec une question d'abord, les nœuds en ligne, les alarmes (les non vues d'abord), une nuit qui l'atténue, une veille après un temps sans toucher, un son et un réveil à l'arrivée d'une nouvelle alarme, et huit langues avec accents, japonais et chinois dessinés avec des polices générées pour exactement les caractères utilisés.
* **Un nœud comme les autres :** le même stockage de réglages, Wi-Fi, page web en huit langues, configuration Bluetooth depuis le téléphone, mise à jour du firmware avec retour arrière et HTTPS comme ARMOR-NODE-RADAR, ARMOR-NODE-SOLAR et ARMOR-NODE-ELECTRICAL (ils partagent `ARMOR-COMMON/firmware_base`), plus des pages pour le serveur, l'écran, le son et la voix ; il signale son existence à Studio par MQTT pour que Studio le liste avec sa page.
* **L'assistant vocal, appuyer pour parler :** un toucher ouvre le micro, le son part seulement vers le service vocal dont l'adresse est dans les réglages (désactivé par défaut), la réponse est affichée et dite ; l'ordre des étapes, les limites de temps et ce que fait un toucher sont une machine à états testée, et le panneau n'exécute aucune commande de lui-même ([le contrat](docs/VOICE.md)). L'enregistrement part en octets dans une requête binaire et la réponse parlée revient en octets ; à un serveur qui ne l'accepte pas, on envoie l'ancienne requête.
* **La carte :** Waveshare ESP32-S3-Touch-LCD-7C-BOX (32 Mo de flash, 16 Mo de PSRAM, écran RGB de 7 pouces avec tactile GT911, quatre micros, un haut-parleur) ; les broches viennent des exemples de Waveshare et un test prouve que deux fonctions ne partagent pas un GPIO ([la carte](docs/HARDWARE.md)).
* **Mises à jour par radio signées :** le nœud n'installe depuis GitHub qu'une image dont le SHA-256 correspond et qui porte une signature Ed25519 valide du responsable (la clé publique est dans le firmware, la privée ne quitte jamais le responsable) ; un fichier envoyé à la main par un administrateur dans le panneau reste accepté. Les pages et messages du panneau sont en huit langues.
* **Appairage Bluetooth avec un PIN :** le PIN à six chiffres est dérivé du code de configuration, imprimé sur la console USB et montré à un administrateur dans le panneau ; le téléphone doit le saisir avant de pouvoir écrire les réglages du nœud.
* **Pas encore :** une première compilation et mise sous tension du firmware, un mot de réveil, le service vocal sur le Jetson et une vue des caméras à l'écran.
* **Seule la maison peut utiliser le panneau :** le panneau répond par défaut au réseau privé (ou aux réseaux que son propriétaire indique, comme un VPN), refuse une liste qui exclurait celui qui l'enregistre, compte les mauvais mots de passe par personne et par adresse, et fonctionne derrière le proxy du serveur pour être atteint depuis l'extérieur sans publier le nœud.
* **Il dit ce qui se passe :** l'écran connaît le texte, en huit langues, de chaque code d'alarme que lève le serveur (réseau, solaire, électrique, zones d'alarme...) et il indique au serveur qu'il est un panneau mural ; il est donc listé comme tel et ne compte pas comme un radar.
* **Liaison chiffrée facultative avec le broker (TLS) :** désactivée par défaut, le nœud parle donc MQTT non chiffré comme avant. Un interrupteur et une zone pour le certificat de l'autorité propre à l'installation (PEM, 2 Ko au plus) dans la page Broker lui font se connecter en `mqtts://`, ne faire confiance qu'à cette autorité et vérifier que le certificat du broker a été établi pour l'adresse atteinte ; ce qui n'est pas un seul certificat PEM est refusé. Couvert par des tests sur ordinateur ; la poignée de main n'a pas encore été exécutée sur une carte.
* **Rapport de santé :** environ une fois par minute le nœud publie depuis combien de temps il tourne et pourquoi il a redémarré la dernière fois, sa mémoire libre, sa liaison et son signal Wi-Fi, combien de fois il s'est connecté au broker et la dernière erreur notée (`armor/health/<nœud>`, le contrat `node_health` d'ARMOR-COMMON). Le code qui le construit est partagé par tous les nœuds et testé sur un ordinateur ; il n'a pas tourné sur une carte.
* **Commande depuis le serveur :** Studio peut redémarrer le nœud proprement, le mettre en pause jusqu'à une heure (il reprend seul), mettre son travail en veille (sans fin, conservée aux redémarrages, le nœud reste connecté et continue de répondre), le faire se signaler, demander un instantané de diagnostic et le marquer en maintenance (`armor/control/<nœud>/...`, le contrat `node_control` d'ARMOR-COMMON). Chaque ordre se fait en deux temps avec un jeton à usage unique propre à cette opération, seul le serveur peut les écrire sur le broker, et il n'existe ni réinitialisation d'usine, ni effacement, ni sommeil profond. Tant que le panneau est en pause ou en veille, l'écran affiche un écran propre qui le dit et ne prend aucun toucher ; quand on lui demande de se signaler, une barre clignote en haut (jamais un son). Le code est partagé par tous les nœuds et testé sur un ordinateur ; il n'a pas tourné sur une carte.
* **Diagnostic sans câble :** après un démarrage qui n'était pas propre (plantage, chien de garde, chute d'alimentation, allocation échouée), le rapport de santé en donne la raison pendant dix minutes ; un plantage laisse son vidage mémoire (core dump) à consulter, télécharger ou effacer depuis le panneau ; les réglages enregistrés sont copiés avant chaque installation de micrologiciel et récupérés si une mise à jour les abîme ; et un administrateur peut demander au nœud, via le serveur, un paquet de diagnostic ou son journal avec un jeton à usage unique.
* **Une sirène qui se répète et des sons dans l'ordre :** tout son passe par une file à priorités (sirène, alarme, voix, confirmation, avis) ; la sirène se répète tant qu'il reste des alarmes critiques non acquittées (`audio.siren_repeat_s` 30, `audio.siren_max_repeats` 10) et les alarmes gardent un volume minimal (`audio.critical_min_volume` 40) ; la voix cède devant une alarme.
* **Une horloge sur batterie :** la puce PCF85063A de la carte donne l'heure au démarrage quand le nœud n'a pas encore d'heure réelle et que la puce est fiable, et elle est corrigée à chaque synchronisation ou réglage de l'heure (`time.rtc_enabled`, activé par défaut).
* **L'état d'un ordre, à l'écran :** le pied de page dit que l'ordre est en cours d'envoi tant qu'il est en route et qu'il a été confirmé pendant quelques secondes lorsqu'il est passé ; un refus y garde sa raison, comme avant.
* **Scellement facultatif des secrets enregistrés :** désactivé par défaut (`security.seal_secrets`, ou `nodes.nvs_encryption` du serveur). Activé, les mots de passe, jetons, clés, codes PIN et empreintes de mots de passe que le nœud enregistre sont scellés avec ChaCha20-Poly1305 et tout le reste reste lisible ; l'activer ou le désactiver les réécrit sans rien perdre, même si le nœud s'arrête en route. Aucun eFuse n'est brûlé et la flash n'est pas chiffrée : la clé est dans une partition de la même puce, donc cela ne protège **pas** contre quelqu'un qui tient la carte, et un nœud installé avant cette partition garde ses secrets tels quels jusqu'à ce qu'il soit flashé avec la nouvelle table de partitions.

## 🧩 Rôle dans A.R.M.O.R.

ARMOR-NODE-HMI est le panneau mural : un client d'[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), comme l'application Android, avec un écran de 7 pouces, un microphone et un haut-parleur. Il montre le mode, les nœuds en ligne et les alarmes qui demandent une personne, arme et désarme (avec le PIN) et acquitte les alarmes. C'est là que vivra l'assistant vocal ([ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)) quand son service vocal existera et, comme tout nœud, il se configure et se met à jour depuis [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO).

| Nom | Comment | À quoi ça sert |
|---|---|---|
| ARMOR-SERVER | HTTPS `GET /api/v1/panel/summary`, WebSocket `/api/v1/panel/events` | Le résumé de la maison (modes, alarmes, nœuds, énergie) et ses changements ; le serveur décide de ce que fait un ordre |
| ARMOR-VOICE-AI | via le serveur | L'audio d'un tour va au service vocal ; la réponse est affichée et dite |
| ARMOR-STUDIO | menu Panneau tactile | Montre la santé du panneau et ouvre son panneau web |
| ARMOR-COMMON | base de firmware | Le firmware commun (panneau, connexions, mise à jour signée, configuration Bluetooth) en est copié |

## 🏛️ Architecture

<p align="center">
  <img src="images/architecture.svg" alt="Architecture de ARMOR-NODE-HMI : qui l'appelle, ce qu'il contient et de quoi il dépend" width="100%">
</p>

## 🖼️ Captures d'écran

Capturées en 1920×1080 sur un banc d'essai, avec des données d'exemple et sans installation réelle derrière.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Résumé : l'état du nœud en un coup d'œil - réseau, broker, firmware et ce qu'il remonte." width="100%"><br><sub>Résumé : l'état du nœud en un coup d'œil - réseau, broker, firmware et ce qu'il remonte.</sub></td><td width="50%" valign="top"><img src="images/screenshots/screen.png" alt="Écran et son : luminosité, heures de nuit, volume et quelles alarmes sont annoncées." width="100%"><br><sub>Écran et son : luminosité, heures de nuit, volume et quelles alarmes sont annoncées.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/voice.png" alt="Assistant vocal : appuyer pour parler, désactivé par défaut, et le service vers lequel va son audio." width="100%"><br><sub>Assistant vocal : appuyer pour parler, désactivé par défaut, et le service vers lequel va son audio.</sub></td><td width="50%" valign="top"><img src="images/screenshots/server.png" alt="Serveur : l'adresse du serveur A.R.M.O.R., l'utilisateur du panneau et la fréquence d'interrogation." width="100%"><br><sub>Serveur : l'adresse du serveur A.R.M.O.R., l'utilisateur du panneau et la fréquence d'interrogation.</sub></td></tr>
</table>

## 🔒 Sécurité et protection

| Menace | Défense |
|---|---|
| Mots de passe en clair ou compilés | PBKDF2-HMAC-SHA256 avec 10 000 tours ; aucun mot de passe dans le firmware (un nœud sans utilisateur passe en configuration) ; au plus 4 utilisateurs (admin ou lecteur) et de 8 à 64 caractères ; au plus 6 sessions, 30 minutes d'inactivité |
| Prendre un nœud tout juste flashé | La configuration exige un code de démarrage : celui du build (`CONFIG_ARMOR_SETUP_CODE`) ou un code aléatoire gardé en flash et affiché sur la console USB ; avec un secret de flotte c'est `HMAC-SHA256(secret, MAC)` ; c'est aussi la clé du Wi-Fi de configuration (WPA2, jamais ponté) |
| Le panneau atteint depuis Internet | Il répond par défaut aux réseaux privés d'une maison (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 et 100.64/10 où vivent certains VPN) ; le réglage `web.admin_sources` liste d'autres adresses et réseaux IPv4 (8 entrées, 120 caractères) ; tout le reste est refusé sauf l'identité publique `GET /api/v1/session` ; une liste qui exclurait celui qui l'enregistre est rejetée |
| Le panneau en clair en HTTP | HTTPS sur le 443 avec un certificat que le nœud fabrique lui-même (autosigné, en flash, empreinte SHA-256 visible pour comparer) ; le panneau peut être http, https ou les deux ; le cookie de session est toujours HttpOnly et SameSite=Strict (et Secure en TLS) |
| Un faux firmware depuis GitHub | Une image n'est installée qu'avec un SHA-256 concordant et une signature Ed25519 valide faite avec la clé du mainteneur (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` la refusent) ; un envoi manuel par un administrateur n'est pas vérifié : une limite explicite |
| Détournement de la configuration Bluetooth | Un PIN à 6 chiffres dérivé du code de configuration, visible sur la console USB et pour un administrateur ; le téléphone doit le saisir avant d'écrire des réglages ; la fenêtre dure `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 par défaut, de 1 à 120) |
| Secrets stockés lus par qui a la carte | Optionnel (désactivé par défaut, interrupteur `security.seal_secrets` ou `nodes.nvs_encryption` du serveur) : ChaCha20-Poly1305 pour mots de passe, jetons, clés, PIN et hachages. Il ne chiffre pas la flash et ne grille aucun eFuse : la clé est dans une partition de la même puce, il ne protège donc pas contre qui tient la carte |
| Le lien avec le broker en clair | TLS optionnel (`mqtts://`) avec l'autorité propre du propriétaire (PEM jusqu'à 2 Ko) et vérification du nom ; désactivé par défaut. Le handshake n'a tourné qu'en tests sur hôte, pas sur une carte |
| L'alarme de contrainte affichée à l'écran | Elle n'est jamais affichée (un test sur hôte le vérifie) ; et ce que le panneau sait du serveur est marqué périmé quand il est ancien |
| Un serveur qui n'est pas le serveur | Le certificat du serveur est épinglé (`server_pin`), pour les requêtes et pour le WebSocket du canal en direct ; le panneau se connecte avec son propre utilisateur, créé par un administrateur, avec le rôle `panel` (il ne peut pas utiliser Studio) |
| Désarmer sans PIN | Armer et désarmer demandent d'abord et prennent le PIN à l'écran (`pin_entry`) ; le serveur le revérifie |
| Audio vocal vers un endroit inattendu | Désactivée par défaut (`voice.enabled` faux) ; l'audio ne va qu'au service dont l'adresse est dans les réglages (l'adresse est validée et changer d'hôte efface le jeton) ; le panneau n'exécute aucun ordre de lui-même et il n'y a pas de mot d'activation : c'est appuyer pour parler |
| Une sirène que personne n'entend | Une seule file de priorité (sirène, alarme, voix, confirmation, avis) ; la sirène se répète tant que des alarmes critiques ne sont pas acquittées (`audio.siren_repeat_s` 30, de 10 à 600, 0 = pas de répétition ; `audio.siren_max_repeats` 10) et le volume critique ne descend jamais sous `audio.critical_min_volume` (40 %) |

Le firmware a été compilé une fois et n'a jamais tourné sur une carte : l'écran, le tactile, le son et la liaison avec le serveur sont écrits d'après les exemples de Waveshare, en attendant la première mise sous tension. Lisez [SECURITY.md](SECURITY.md) pour signaler une vulnérabilité.

## 🔌 Interfaces

| Partie | Ce qu'elle règle |
|---|---|
| ARMOR-SERVER | `GET /api/v1/panel/summary` toutes les `server.poll_s` (3 s, de 1 à 60) seulement quand le canal en direct est tombé, et le WebSocket `/api/v1/panel/events` tant que `server.live_events` est vrai (par défaut) : le résumé arrive quand le mode, les alarmes, les nœuds ou l'énergie changent ; l'utilisateur propre du panneau a le rôle `panel` |
| Voix | L'audio d'un tour va à ARMOR-VOICE-AI via le serveur (`/api/v1/voice/turn`, avec un corps multipart facultatif) ; `core/voice_session.hpp` est la machine d'états (ordre des étapes, limites de temps, ce que fait un toucher) ; voir [docs/VOICE.md](docs/VOICE.md) |
| Matériel | Waveshare ESP32-S3-Touch-LCD-7C-BOX : écran RGB 7 pouces 800x480, tactile GT911, quatre microphones, un haut-parleur, RTC PCF85063A (`time.rtc_enabled`, vrai par défaut), 32 Mo de flash et 16 Mo de PSRAM ; un test prouve qu'aucune fonction ne partage un GPIO |

## ⚙️ Configuration

| Variable | Par défaut | Ce qu'elle règle |
|---|---|---|
| `server.poll_s`, `server.live_events` | 3 (1-60), true | La période d'interrogation du résumé et le canal en direct par WebSocket |
| `display.sleep_s` | 300 (0-3600) | Secondes avant la mise en veille de l'écran (0 = jamais) ; la luminosité et la luminosité de nuit sont aussi des réglages |
| `audio.critical_min_volume`, `audio.siren_repeat_s`, `audio.siren_max_repeats` | 40 (0-100), 30, 10 | Le volume minimal d'un son critique, la période de répétition de la sirène (0 ou de 10 à 600 s) et ses répétitions maximales |
| `audio.tone_by_source` | selon la gravité | Le son de chaque origine d'alarme (appareil, solaire, électrique, réseau, centrale, autre) : celui que dicte sa gravité, en avis, en alarme, en sirène sans son, `spoken` (l'alarme dite à voix haute) ou `custom` (le son téléversé pour cette origine), tous deux produits par le serveur et joués seulement s'ils arrivent entiers, sinon le son habituel ; un son choisi retentit même sous la gravité minimale |
| `audio.clip_max_bytes` | 200000 | Le maximum d'audio, en octets de 16 kHz, 16 bits, mono (de 3200 à 262144), pris au serveur pour une alarme dite à voix haute ou un son personnalisé ; une réponse plus longue n'est pas jouée |
| `voice.enabled`, `voice.url`, `voice.token` | false, "", - | Le service vocal : désactivé par défaut ; l'adresse fait au plus 160 caractères et est validée (sinon `required` ou `invalid`) ; le jeton est effacé quand l'hôte change |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | Les réglages que tous les nœuds partagent : le nom (vide donne `armor-` et les derniers octets de la MAC), le code de configuration (vide : aléatoire), le secret de flotte (16+ caractères, jamais dans le dépôt), les premiers réglages du broker, le battement (de 2 à 300 s), les minutes de configuration Bluetooth (de 1 à 120) et la réinitialisation avec BOOT (8 s dans les 30 premières s) |

## 🚀 Démarrage rapide

**Prérequis:** un compilateur C++17 pour les tests sur hôte ; le conteneur ESP-IDF 5.5.5 pour construire l'image ; une Waveshare ESP32-S3-Touch-LCD-7C-BOX.

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel of the node without a board
tools/build_node.sh                                   # the image in the ESP-IDF container
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

Voir la [conception](docs/DESIGN.md), la [carte](docs/HARDWARE.md), l'[assistant vocal](docs/VOICE.md), le [guide du firmware](docs/NODE_FIRMWARE.md) et les [polices](docs/FONTS.md).

Pour compiler le firmware à la main dans GitHub Actions, voir [compiler dans GitHub Actions](docs/FIRMWARE_BUILD_ACTIONS.md).

## 🧪 Tests

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 programmes de test sur hôte et 6 contrôles Python tournent sous `ctest` ; 93122 vérifications comptées par les programmes hôte, compilés avec `-Werror`.

Contrôles Python : les textes du panneau en huit langues, les tâches, les chemins de sauvegarde, la session Bluetooth sous son verrou et les routes appelées par le panneau (et, dans certains nœuds, les codes HTTP ou les glyphes des polices)

Les tests sur hôte demandent un compilateur C++17 quelconque (Linux, WSL, MSYS2) ; le panneau web est aussi exercé dans un vrai navigateur contre un nœud de substitution (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

`check_font_glyphs.py` vérifie que les polices ont tous les glyphes des huit langues (98 lignes) ; `test_clock` n'imprime que `ok`.

## 🏗️ Compilation et publication

`tools/build_node.sh` construit l'image dans le conteneur ESP-IDF 5.5.5 et la version porte l'image, son SHA-256 et sa signature Ed25519, une par carte ; l'image est écrite par USB-C la première fois, puis mise à jour depuis le panneau.

## 🩺 Dépannage

| Vous voyez | Parce que | Faites |
|---|---|---|
| Vous avez perdu le mot de passe ou le nœud ne rejoint plus le réseau | Un nœud en configuration sans adresse après 90 s ouvre son propre Wi-Fi `ARMOR-SETUP-xxxxxx` (ouvrez `http://192.168.4.1/`) | Pour recommencer, maintenez BOOT 8 secondes dans les 30 premières secondes après la mise sous tension (efface réglages et utilisateurs), ou utilisez *Réinitialisation d'usine* dans le panneau |
| Une mise à jour depuis GitHub est refusée | La version n'a ni somme de contrôle ni signature, elles ne concordent pas (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), n'a pas d'image pour cette carte (`e_no_asset_for_board`) ou est d'un autre projet (`e_wrong_firmware`) | Le nœud n'a pas changé ; réessayez ou laissez un administrateur envoyer le fichier à la main |
| La voix ne démarre pas | `voice.enabled` est désactivé, ou `voice.url` est `required` ou `invalid` | Activez-la dans le panneau et donnez une adresse valide ; changer d'hôte efface le jeton |
| Le pied d'écran dit envoi, puis confirmée ou le motif | Un ordre est en route, a été confirmé quelques secondes, ou refusé et garde son motif | Lisez le motif ; c'est le serveur qui décide, pas le panneau |

## 📂 Structure du dépôt

<p align="center">
  <img src="images/repository-structure.svg" alt="Structure du dépôt de ARMOR-NODE-HMI" width="100%">
</p>

<details>
<summary>Version texte de l'arborescence</summary>

```text
ARMOR-NODE-HMI/
├── main/                                le firmware : un composant ESP-IDF
├── core/                                la logique sans ESP-IDF, testée sur ordinateur
├── panel/                               le panneau web intégré au firmware
├── tests/                               tests sur hôte et contrôles Python
├── docs/                                9 guides : conception, voix, polices, matériel
├── tools/                               outils de build, d'empaquetage et de contrôle
├── .github/                             workflow CI et modèles d'issues et de pull requests
├── images/                              bannière, icône et les deux images de ce README
├── build.sh build.bat                   build par le flux de projet partagé
├── test.sh test.bat                     lancer les tests par le flux partagé
├── run.sh run.bat                       démarrer par le flux de projet partagé
├── armor.project.json                   manifeste : version, rôle, maturité, commande de build
├── CHANGELOG.md                         ce qui a changé à chaque version
├── SECURITY.md SUPPORT.md LICENSE       politique de sécurité, support et licence
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   comment contribuer et le code de conduite
├── CMakeLists.txt                       le projet ESP-IDF
├── partitions.csv                       la table de la flash
├── sdkconfig.defaults                   les réglages du firmware
├── sdkconfig.board.lcd7box              les réglages d'une carte
├── secrets/                             exemple du fichier de nœud (les vrais ne sont pas versionnés)
├── dependencies.lock                    les versions épinglées des composants ESP-IDF
├── build-test.sh build-test.bat         lancer les tests par le flux partagé
└── .gitattributes .gitignore            réglages Git
```

</details>

## 🗺️ Feuille de route et limites connues

**Limites connues aujourd'hui**

* Le firmware a été compilé une fois (ESP-IDF 5.5.5), mais la compilation n'est pas contrôlée par l'intégration continue et il n'a jamais tourné sur une carte.
* L'échange avec le serveur et avec le service vocal ARMOR-VOICE-AI est testé contre des simulations ; c'est du push-to-talk, sans mot d'activation.
* La page web a tourné contre une simulation dans un vrai navigateur ; l'écran, le tactile et le son sont écrits d'après les exemples de Waveshare et attendent leur première mise sous tension.
* Le firmware n'a pas tourné sur la carte : rien n'a encore tourné sur de vrais équipements.
* Le code de configuration, la limite de connexions, le PIN Bluetooth, la gestion de la contrainte et les mises à jour signées sont testés sur l'hôte ; il n'y a pas eu de revue indépendante.
* Les contrôles de hash et de signature réussissent sur l'hôte ; une mise à jour par voie hertzienne n'a jamais été exécutée sur une carte.
* Les messages sont validés par rapport aux schémas partagés d'ARMOR-COMMON ; la liaison avec le serveur et avec le service vocal n'a pas été essayée depuis une carte.

**Ensuite**

* Faire la première compilation complète et la première mise sous tension sur la carte, puis le service vocal sur le Jetson, un mot d'activation et la vue des caméras à l'écran.

## 🌍 Langues

Le README et les textes existent en anglais, espagnol, français, italien, allemand, chinois, japonais et portugais européen.

## 🔗 Projets liés

**A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) est un système de sécurité périmétrique composé de dépôts indépendants. Chacun a sa propre version, ses propres tests et son propre README ; voici la famille :

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contrats de messages, validateurs, vecteurs de conformité et types générés
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware du nœud de terrain pour ESP32-S3 avec trois radars et son propre panneau web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocoles des onduleurs et batteries solaires et messages d'un nœud passerelle
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Nœud électrique : compteurs, le message des mesures du réseau et les règles de commutation
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nœud et centrale d'alarme : zones, armement, temporisations, sirène et PIN, avec ou sans le serveur
* **ARMOR-NODE-HMI** (ce dépôt) - Panneau tactile : l'état du système sur un écran mural, armer et acquitter, et la maison de l'assistant vocal
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - Le réseau local : ses appareils, internet et ce qui change
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordinateur central : télémétrie, alarmes, appareils, relevés solaires et caméras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Console web : caméras, radar, alarmes, énergie solaire et concepteur de site 2D/3D
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Client Android de l'opérateur avec radar 2D/3D en direct
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Client opérateur iPhone
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Politique d'inférence visuelle qui explique ses décisions et n'agit jamais
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zones de caméra, masques et règles de détection pour la Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intentions vocales hors ligne avec une confirmation impossible à falsifier
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Boîtiers, électronique et matrice d'acceptation sur banc
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Déploiement, banc d'essai CM5, sauvegarde et TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulateur de télémétrie hors ligne avec des pannes reproductibles
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Détecte, installe et met à jour les propres dépôts de l'écosystème
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Architecture, base de sécurité et matrice des capacités

## 📚 Documentation

* [Conception](docs/DESIGN.md) et [matériel](docs/HARDWARE.md) : ce que montre le panneau et la carte
* [Voix](docs/VOICE.md) et [polices](docs/FONTS.md)
* [Firmware du nœud](docs/NODE_FIRMWARE.md) et [actions de build](docs/FIRMWARE_BUILD_ACTIONS.md)
* [Essais de banc](docs/BENCH_TESTS.md), [code de configuration](docs/SETUP_CODE.md) et [mise en service Bluetooth](docs/BLE_PROVISIONING.md)
* [Matrice des capacités : ce qui est prouvé et ce qui ne l'est pas](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catalogue des projets : versions et dépendances entre les dépôts](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Historique des modifications de ce dépôt](CHANGELOG.md)

## 🤝 Contribuer et communauté

* Avant une pull request, lancez `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py` : il doit réussir. Voir [CONTRIBUTING.md](CONTRIBUTING.md) et le [code de conduite](CODE_OF_CONDUCT.md).
* Le README existe en huit langues et est généré à partir des sources d'ARMOR-DOCS : une modification du texte se fait là-bas.
* Une modification d'un message entre composants commence dans ARMOR-COMMON.
* Ne décrivez pas comme fonctionnant sur une carte ce qui a seulement été compilé.
* Signalez une vulnérabilité en privé : [SECURITY.md](SECURITY.md). Questions, idées et rapports : electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTEUR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENCE

GPL-3.0-or-later - voir [LICENSE](LICENSE).
