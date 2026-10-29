<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-HMI banner" width="100%">
</p>

# 🖥️ ARMOR-NODE-HMI

<p align="center">
  <a href="README.md">🇺🇸 English</a> |
  <a href="README_spa.md">🇪🇸 Español</a> |
  <a href="README_fra.md">🇫🇷 Français</a> |
  <a href="README_ita.md">🇮🇹 Italiano</a> |
  <a href="README_deu.md">🇩🇪 Deutsch</a> |
  <a href="README_zho.md">🇨🇳 简体中文</a> |
  <a href="README_jpn.md">🇯🇵 日本語</a> |
  🇵🇹 <b>Português</b>
</p>

### O painel tátil da casa: um ecrã de 7 polegadas na parede que mostra o estado do sistema, arma e desarma e reconhece alarmes, com um microfone e um altifalante para o assistente de voz que virá (um nó da família na ESP32-S3-Touch-LCD-7C-BOX, com a sua própria página web para as definições)

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

**Verificação de honestidade - o que funciona hoje:** O que não depende da placa é real e está testado num computador (as definições, o que o painel sabe do servidor e quando está desatualizado, o alarme de coação que nunca é mostrado, a conversa de voz e a resposta longa do serviço de voz, os pinos da placa, os textos do ecrã em oito idiomas e os glifos das suas fontes, as verificações de versão e assinatura: 30,783 verificações em 19 programas de teste), e a página web também (executada contra um substituto num navegador real). **O firmware foi compilado uma vez (ESP-IDF 5.5.5, não verificado pela integração contínua) e nunca correu numa placa:** o ecrã, o toque, o som e a ligação ao servidor estão escritos a partir dos exemplos da Waveshare para esta família de placas e precisam do primeiro arranque. O serviço de voz (ARMOR-VOICE-AI) responde aos pedidos deste painel; não há palavra de ativação: toca-se para falar.

---

## 📋 Num relance

| | |
|---|---|
| Versão | `0.4.0` |
| Maturidade | Funcional |
| Função | Painel tátil de parede para o ARMOR-SERVER (família: Clients, pai: ARMOR-SERVER) |
| Tecnologia | C++17, ESP-IDF 5.5.5, LVGL, ESP32-S3 |
| Testes | 93122 verificações em 19 programas de anfitrião e 6 em Python (`ctest`) |
| Placa | Waveshare ESP32-S3-Touch-LCD-7C-BOX (800x480, GT911) |
| Funciona em | Uma Waveshare ESP32-S3-Touch-LCD-7C-BOX; testes de anfitrião em qualquer computador |
| Em hardware real | Nenhum ainda: o firmware compila (ESP-IDF 5.5.5, não verificado pela CI) mas não correu na placa |
| Licença | GPL-3.0-or-later |

**Ir para:** [Visão geral](#-visão-geral) · [Arquitetura](#-arquitetura) · [Segurança](#-segurança-e-proteção) · [Interfaces](#-interfaces) · [Configuração](#-configuração) · [Início rápido](#-início-rápido) · [Testes](#-testes) · [Problemas](#-resolução-de-problemas) · [Estrutura](#-estrutura-do-repositório) · [Relacionados](#-projetos-relacionados)

## 🚦 Estado do projeto

Cada linha é uma parte deste repositório e a luz diz até onde chega hoje. É escrita com o que o repositório consegue demonstrar (testes, compilações, execuções), não com o que está previsto.

🟢 Feito e verificado · 🟡 Funciona, com limites conhecidos · 🟠 Feito a meio ou ainda por verificar · 🔴 Por fazer ou a não funcionar

| Área | Estado | O que significa aqui |
|---|---|---|
| Software | 🟠 | O firmware foi compilado uma vez (ESP-IDF 5.5.5), mas a compilação não é verificada pela integração contínua e nunca funcionou numa placa. |
| Testes | 🟢 | Passam 93.122 verificações em 19 programas de anfitrião (`ctest`): as definições, o que o painel sabe do servidor e quando isso está desatualizado, o alarme de coação que nunca se mostra, a troca de voz, os pinos, os textos do ecrã em oito idiomas e os glifos dos seus tipos de letra. |
| Protocolos e dados | 🟡 | A troca com o servidor e com o serviço de voz ARMOR-VOICE-AI é testada contra simulações; é premir para falar, sem palavra de ativação. |
| Interface | 🟠 | A página web correu contra uma simulação num navegador real; o ecrã, o toque e o som foram escritos a partir dos exemplos da Waveshare e esperam a primeira ligação. |
| Hardware | 🔴 | O firmware não funcionou na placa: nada funcionou ainda em equipamentos reais. |
| Segurança | 🟡 | O código de configuração, o limite de acessos, o PIN Bluetooth, o tratamento da coação e as atualizações assinadas estão testados no anfitrião; não houve revisão independente. |
| Atualizações | 🟠 | As verificações de hash e de assinatura passam no anfitrião; uma atualização pelo ar nunca foi executada numa placa. |
| Integração | 🟡 | As mensagens são validadas contra os esquemas partilhados do ARMOR-COMMON; a ligação ao servidor e ao serviço de voz não foi experimentada a partir de uma placa. |
| Documentação | 🟢 | Oito idiomas, o guia do firmware, as notas de cablagem e o registo de alterações estão atualizados. |

## 🎯 Visão geral

* **Um cliente do ARMOR-SERVER, como a aplicação Android:** inicia sessão com um utilizador que um administrador criou para ele, pede `GET /api/v1/panel/summary` (algumas centenas de bytes: o modo, os nós ligados e os alarmes que precisam de uma pessoa) a cada poucos segundos e, quando a pessoa toca no botão e confirma, arma, desarma ou reconhece. Se o servidor deixa de responder, o ecrã diz que o que mostra é antigo e o botão fica desativado. Ouve o canal em direto do servidor (um WebSocket) para que uma alteração se veja de imediato, e pergunta como antes quando o canal falta, se cala ou está desligado.
* **O ecrã (800x480, LVGL):** o modo na cor do pior alarme que ninguém viu, um grande botão de armar/desarmar com uma pergunta primeiro, os nós ligados, os alarmes (os não vistos primeiro), uma noite que o escurece, um repouso após algum tempo sem toques, um som e um despertar quando chega um alarme novo, e oito idiomas com acentos, japonês e chinês desenhados com tipos de letra gerados exatamente para os carateres usados.
* **Um nó como os outros:** o mesmo armazenamento de definições, Wi-Fi, página web em oito idiomas, configuração por Bluetooth a partir do telemóvel, atualização de firmware com retrocesso e HTTPS como o ARMOR-NODE-RADAR, o ARMOR-NODE-SOLAR e o ARMOR-NODE-ELECTRICAL (partilham `ARMOR-COMMON/firmware_base`), mais páginas para o servidor, o ecrã, o som e a voz; diz ao Studio que existe por MQTT para que o Studio o liste com a sua página.
* **O assistente de voz, premir para falar:** um toque abre o microfone, o áudio vai só para o serviço de fala cujo endereço está nas definições (desativado por omissão), a resposta mostra-se e diz-se em voz alta; a ordem dos passos, os limites de tempo e o que faz um toque são uma máquina de estados testada, e o painel não executa nenhum comando por si só ([o contrato](docs/VOICE.md)). A gravação segue como bytes num pedido binário e a resposta falada volta como bytes; a um servidor que não a aceite envia-se o pedido antigo.
* **A placa:** Waveshare ESP32-S3-Touch-LCD-7C-BOX (32 MB de flash, 16 MB de PSRAM, um ecrã RGB de 7 polegadas com toque GT911, quatro microfones, um altifalante); os pinos vêm dos exemplos da Waveshare e um teste prova que nenhumas duas funções partilham um GPIO ([a placa](docs/HARDWARE.md)).
* **Atualizações pelo ar assinadas:** o nó só instala do GitHub uma imagem cujo SHA-256 coincide e que leva uma assinatura Ed25519 válida do responsável (a chave pública está no firmware, a privada nunca sai do responsável); um ficheiro carregado à mão por um administrador no painel continua a ser aceite. As páginas e mensagens do painel estão em oito idiomas.
* **Emparelhamento Bluetooth com PIN:** o PIN de seis dígitos deriva do código de configuração, é impresso na consola USB e mostrado a um administrador no painel; o telemóvel tem de o introduzir antes de poder escrever as definições do nó.
* **Ainda não:** uma primeira compilação e arranque do firmware, uma palavra de ativação, o serviço de fala no Jetson, e uma vista das câmaras no ecrã.
* **Só a casa pode usar o painel:** o painel responde por omissão à rede privada (ou às redes que o dono indicar, como uma VPN), recusa uma lista que deixaria de fora quem a guarda, conta as palavras-passe erradas por pessoa e por endereço e funciona atrás do proxy do servidor para ser alcançado de fora de casa sem publicar o nó.
* **Diz o que se passa:** o ecrã conhece o texto, em oito idiomas, de cada código de alarme que o servidor levanta (rede, solar, elétrica, zonas de alarme...) e avisa o servidor de que é um painel de parede, por isso é listado como tal e não conta como radar.
* **Ligação cifrada opcional ao broker (TLS):** desativada por predefinição, por isso o nó fala MQTT sem cifra como antes. Um interruptor e uma caixa para o certificado da autoridade própria da instalação (PEM, no máximo 2 KB) na página Broker fazem-no ligar-se com `mqtts://`, confiar só nessa autoridade e verificar que o certificado do broker foi feito para o endereço alcançado; o que não for um único certificado PEM é recusado. Coberto por testes no computador; o handshake ainda não foi executado numa placa.
* **Relatório de saúde:** cerca de uma vez por minuto o nó publica há quanto tempo está ligado e porque reiniciou da última vez, a memória livre, a ligação e o sinal Wi-Fi, quantas vezes se ligou ao broker e o último erro que registou (`armor/health/<nó>`, o contrato `node_health` do ARMOR-COMMON). O código que o constrói é partilhado por todos os nós e testado num computador; nunca correu numa placa.
* **Controlo a partir do servidor:** o Studio pode reiniciar o nó de forma limpa, pô-lo em pausa até uma hora (retoma sozinho), pôr o seu trabalho em espera (sem fim, mantida nos reinícios, o nó continua ligado e a responder), fazê-lo mostrar-se, pedir um instantâneo de diagnóstico e marcá-lo em manutenção (`armor/control/<nó>/...`, o contrato `node_control` do ARMOR-COMMON). Cada ordem tem dois passos com um token de uso único que pertence a essa operação, só o servidor as pode escrever no broker, e não existe reposição de fábrica, apagamento nem sono profundo. Enquanto o painel está em pausa ou em espera, o ecrã mostra um ecrã próprio que o diz e não aceita toques; quando lhe pedem que se identifique, pisca uma barra no topo (nunca um som). O código é partilhado por todos os nós e testado num computador; ainda não correu numa placa.
* **Diagnóstico sem cabo:** depois de um arranque que não foi limpo (um bloqueio, um watchdog, uma quebra de alimentação, uma reserva de memória falhada), o relatório de saúde diz o motivo durante dez minutos; uma falha deixa o seu core dump para ver, descarregar ou apagar a partir do painel; as definições guardadas são copiadas antes de cada instalação de firmware e recuperadas se uma atualização as danificar; e um administrador pode pedir ao nó, através do servidor, um pacote de diagnóstico ou o seu registo com um token de utilização única.
* **Uma sirene que se repete e sons por ordem:** todo o som passa por uma fila com prioridades (sirene, alarme, voz, confirmação, aviso); a sirene repete-se enquanto houver alarmes críticos por reconhecer (`audio.siren_repeat_s` 30, `audio.siren_max_repeats` 10) e os alarmes mantêm um volume mínimo (`audio.critical_min_volume` 40); a voz cede perante um alarme.
* **Um relógio com bateria:** o chip PCF85063A da placa dá a hora no arranque quando o nó ainda não tem hora real e o chip é de confiança, e é corrigido sempre que a hora é sincronizada ou acertada (`time.rtc_enabled`, ativado por predefinição).
* **O estado de uma ordem, no ecrã:** o rodapé diz que a ordem está a ser enviada enquanto vai a caminho e que foi confirmada durante alguns segundos quando passou; uma recusa mantém aí o seu motivo, como antes.
* **Selagem opcional dos segredos guardados:** desativada por predefinição (`security.seal_secrets`, ou `nodes.nvs_encryption` do servidor). Ativada, as palavras-passe, os tokens, as chaves, os PIN e os hashes de palavras-passe que o nó guarda ficam selados com ChaCha20-Poly1305 e tudo o resto continua legível; ativá-la ou desativá-la reescreve-os sem perder nada, mesmo que o nó pare a meio. Não se queima nenhum eFuse nem se cifra a flash: a chave está numa partição do mesmo chip, pelo que **não** protege contra quem tenha a placa, e um nó instalado antes dessa partição mantém os segredos como estavam até ser gravado com a nova tabela de partições.

## 🧩 Papel no A.R.M.O.R.

O ARMOR-NODE-HMI é o painel de parede: um cliente do [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER), como a aplicação Android, com um ecrã de 7 polegadas, microfone e altifalante. Mostra o modo, os nós ligados e os alarmes que precisam de uma pessoa, arma e desarma (com o PIN) e reconhece alarmes. É onde viverá o assistente de voz ([ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)) quando existir o seu serviço de voz e, como todos os nós, é configurado e atualizado a partir do [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO).

| Nome | Como | Para quê |
|---|---|---|
| ARMOR-SERVER | HTTPS `GET /api/v1/panel/summary`, WebSocket `/api/v1/panel/events` | O resumo da casa (modos, alarmes, nós, energia) e as suas mudanças; o servidor decide o que faz uma ordem |
| ARMOR-VOICE-AI | através do servidor | O áudio de um turno vai para o serviço de voz; a resposta é mostrada e dita |
| ARMOR-STUDIO | menu Painel tátil | Mostra a saúde do painel e abre o seu painel web |
| ARMOR-COMMON | base de firmware | O firmware comum (painel, acessos, atualização assinada, configuração Bluetooth) é copiado dele |

## 🏛️ Arquitetura

<p align="center">
  <img src="images/architecture.svg" alt="Arquitetura de ARMOR-NODE-HMI: quem o chama, o que contém e de que depende" width="100%">
</p>

## 🖼️ Capturas de ecrã

Capturadas a 1920×1080 numa bancada de testes, com dados de exemplo e sem qualquer instalação real por trás.

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="Resumo: o estado do nó num relance - rede, broker, firmware e o que reporta." width="100%"><br><sub>Resumo: o estado do nó num relance - rede, broker, firmware e o que reporta.</sub></td><td width="50%" valign="top"><img src="images/screenshots/screen.png" alt="Ecrã e som: brilho, horas noturnas, volume e que alarmes são anunciados." width="100%"><br><sub>Ecrã e som: brilho, horas noturnas, volume e que alarmes são anunciados.</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/voice.png" alt="Assistente de voz: premir para falar, desativado por predefinição, e o serviço para onde vai o seu áudio." width="100%"><br><sub>Assistente de voz: premir para falar, desativado por predefinição, e o serviço para onde vai o seu áudio.</sub></td><td width="50%" valign="top"><img src="images/screenshots/server.png" alt="Servidor: o endereço do servidor A.R.M.O.R., o utilizador do painel e de quanto em quanto tempo pergunta." width="100%"><br><sub>Servidor: o endereço do servidor A.R.M.O.R., o utilizador do painel e de quanto em quanto tempo pergunta.</sub></td></tr>
</table>

## 🔒 Segurança e proteção

| Ameaça | Defesa |
|---|---|
| Palavras-passe em claro ou compiladas | PBKDF2-HMAC-SHA256 com 10 000 rondas; nenhuma palavra-passe no firmware (um nó sem utilizador entra em configuração); no máximo 4 utilizadores (admin ou visualizador) e de 8 a 64 caracteres; no máximo 6 sessões, 30 minutos de inatividade |
| Tomar um nó acabado de gravar | A configuração exige um código de arranque: o da compilação (`CONFIG_ARMOR_SETUP_CODE`) ou um aleatório guardado na flash e mostrado na consola USB; com segredo de frota é `HMAC-SHA256(segredo, MAC)`; é também a chave do Wi-Fi de configuração (WPA2, nunca em ponte) |
| O painel alcançado a partir da internet | Responde por omissão às redes privadas de uma casa (10/8, 172.16/12, 192.168/16, 169.254/16, 127/8 e 100.64/10 onde vivem algumas VPN); a definição `web.admin_sources` lista outros endereços e redes IPv4 (8 entradas, 120 caracteres); tudo o que está fora é recusado exceto a identidade pública `GET /api/v1/session`; uma lista que deixaria de fora quem a guarda é rejeitada |
| O painel em claro por HTTP | HTTPS no 443 com um certificado que o nó cria sozinho (autoassinado, na flash, impressão digital SHA-256 visível para comparar); o painel pode ser http, https ou ambos; o cookie de sessão é sempre HttpOnly e SameSite=Strict (e Secure por TLS) |
| Um firmware falso do GitHub | Uma imagem só se instala com um SHA-256 coincidente e uma assinatura Ed25519 válida feita com a chave do mantenedor (`no_checksum`, `no_signature`, `bad_signature`, `checksum_mismatch`, `wrong_firmware`, `not_newer`, `version_mismatch` recusam-na); um envio manual por um administrador não é verificado: um limite explícito |
| Sequestro da configuração Bluetooth | Um PIN de 6 dígitos derivado do código de configuração, visível na consola USB e ao administrador; o telemóvel tem de o introduzir antes de escrever definições; a janela dura `CONFIG_ARMOR_BLE_SETUP_MINUTES` (10 por omissão, de 1 a 120) |
| Segredos guardados lidos por quem tem a placa | Opcional (desligado por omissão, interruptor `security.seal_secrets` ou `nodes.nvs_encryption` do servidor): ChaCha20-Poly1305 para palavras-passe, tokens, chaves, PIN e hashes. Não cifra a flash nem queima um eFuse: a chave está numa partição do mesmo chip, por isso não protege contra quem tem a placa |
| A ligação ao broker em claro | TLS opcional (`mqtts://`) com a autoridade própria do dono (PEM até 2 KB) e verificação do nome; desligado por omissão. O handshake só correu em testes de anfitrião, não numa placa |
| O alarme de coação mostrado no ecrã | Nunca é mostrado (um teste de anfitrião verifica-o); e o que o painel sabe do servidor é marcado como caducado quando é antigo |
| Um servidor que não é o servidor | O certificado do servidor está fixado (`server_pin`), para os pedidos e para o WebSocket do canal em direto; o painel entra com um utilizador próprio, criado por um administrador, com o papel `panel` (não pode usar o Studio) |
| Desarmar sem PIN | Armar e desarmar perguntam primeiro e recebem o PIN no ecrã (`pin_entry`); o servidor volta a verificar |
| Áudio de voz para um sítio inesperado | Desligada por omissão (`voice.enabled` falso); o áudio só vai para o serviço cujo endereço está nas definições (o endereço é validado e mudar de anfitrião apaga o token); o painel não executa nenhuma ordem por si e não há palavra de ativação: é premir para falar |
| Uma sirene que ninguém ouve | Uma única fila de prioridade (sirene, alarme, voz, confirmação, aviso); a sirene repete-se enquanto houver alarmes críticos por reconhecer (`audio.siren_repeat_s` 30, de 10 a 600, 0 = sem repetições; `audio.siren_max_repeats` 10) e o volume crítico nunca desce de `audio.critical_min_volume` (40 %) |

O firmware foi compilado uma vez e nunca correu numa placa: o ecrã, o tátil, o som e a ligação ao servidor estão escritos a partir dos exemplos da Waveshare, à espera do primeiro arranque. Leia [SECURITY.md](SECURITY.md) para comunicar uma vulnerabilidade.

## 🔌 Interfaces

| Parte | O que define |
|---|---|
| ARMOR-SERVER | `GET /api/v1/panel/summary` a cada `server.poll_s` (3 s, de 1 a 60) só enquanto o canal em direto está em baixo, e o WebSocket `/api/v1/panel/events` enquanto `server.live_events` for verdadeiro (predefinição): o resumo chega quando mudam o modo, os alarmes, os nós ou a energia; o utilizador próprio do painel tem o papel `panel` |
| Voz | O áudio de um turno vai para o ARMOR-VOICE-AI através do servidor (`/api/v1/voice/turn`, com corpo multipart opcional); `core/voice_session.hpp` é a máquina de estados (ordem dos passos, limites de tempo, o que faz um toque); veja [docs/VOICE.md](docs/VOICE.md) |
| Hardware | Waveshare ESP32-S3-Touch-LCD-7C-BOX: ecrã RGB de 7 polegadas 800x480, tátil GT911, quatro microfones, um altifalante, RTC PCF85063A (`time.rtc_enabled`, verdadeiro por omissão), 32 MB de flash e 16 MB de PSRAM; um teste prova que nenhuma função partilha um GPIO |

## ⚙️ Configuração

| Variável | Predefinição | O que define |
|---|---|---|
| `server.poll_s`, `server.live_events` | 3 (1-60), true | O período de consulta do resumo e o canal em direto por WebSocket |
| `display.sleep_s` | 300 (0-3600) | Segundos até o ecrã adormecer (0 = nunca); o brilho e o brilho noturno também são definições |
| `audio.critical_min_volume`, `audio.siren_repeat_s`, `audio.siren_max_repeats` | 40 (0-100), 30, 10 | O volume mínimo de um som crítico, o período de repetição da sirene (0 ou de 10 a 600 s) e as suas repetições máximas |
| `audio.tone_by_source` | conforme a gravidade | O som de cada origem de alarme (dispositivo, solar, elétrico, rede, central, outro): o que a sua gravidade indica, como aviso, como alarme, como sirene sem som, `spoken` (o alarme dito em voz alta) ou `custom` (o som carregado para essa origem), ambos feitos pelo servidor e reproduzidos só se chegarem inteiros, senão o tom habitual; um som escolhido toca mesmo abaixo da gravidade mínima |
| `audio.clip_max_bytes` | 200000 | O máximo de áudio, em bytes de 16 kHz, 16 bits, mono (de 3200 a 262144), obtido do servidor para um alarme dito em voz alta ou um som próprio; uma resposta mais longa não é reproduzida |
| `voice.enabled`, `voice.url`, `voice.token` | false, "", - | O serviço de voz: desligado por omissão; o endereço tem no máximo 160 caracteres e é validado (senão `required` ou `invalid`); o token apaga-se quando o anfitrião muda |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | As definições que todos os nós partilham: o nome (vazio dá `armor-` e os últimos bytes do MAC), o código de configuração (vazio: aleatório), o segredo de frota (16+ caracteres, nunca no repositório), as primeiras definições do broker, o batimento (de 2 a 300 s), os minutos de configuração Bluetooth (de 1 a 120) e a reposição com BOOT (8 s nos primeiros 30 s) |

## 🚀 Início rápido

**Requisitos:** um compilador C++17 para os testes de anfitrião; o contentor ESP-IDF 5.5.5 para compilar a imagem; uma Waveshare ESP32-S3-Touch-LCD-7C-BOX.

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel of the node without a board
tools/build_node.sh                                   # the image in the ESP-IDF container
```

Veja o [guia do firmware](docs/NODE_FIRMWARE.md) e o [canal Bluetooth](docs/BLE_PROVISIONING.md).

Veja o [desenho](docs/DESIGN.md), a [placa](docs/HARDWARE.md), o [assistente de voz](docs/VOICE.md), o [guia do firmware](docs/NODE_FIRMWARE.md) e os [tipos de letra](docs/FONTS.md).

Para compilar o firmware à mão no GitHub Actions, veja [compilar no GitHub Actions](docs/FIRMWARE_BUILD_ACTIONS.md).

## 🧪 Testes

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

19 programas de teste de anfitrião e 6 verificações Python correm com `ctest`; 93122 verificações contadas pelos programas de anfitrião, compiladas com `-Werror`.

Verificações Python: os textos do painel em oito línguas, as tarefas, os caminhos de gravação, a sessão Bluetooth sob o seu bloqueio e as rotas que o painel chama (e, em alguns nós, os códigos HTTP ou os glifos dos tipos de letra)

Os testes de anfitrião precisam de qualquer compilador C++17 (Linux, WSL, MSYS2); o painel web também é exercitado num navegador real contra um nó de substituição (`tools/panel_mock.mjs`, `tools/panel_browser_test.mjs`).

O `check_font_glyphs.py` verifica que os tipos de letra têm todos os glifos das oito línguas (98 linhas); o `test_clock` só imprime `ok`.

## 🏗️ Compilação e publicação

`tools/build_node.sh` compila a imagem no contentor ESP-IDF 5.5.5 e a versão leva a imagem, o seu SHA-256 e a sua assinatura Ed25519, uma por placa; a imagem grava-se por USB-C da primeira vez e depois atualiza-se a partir do painel.

## 🩺 Resolução de problemas

| Vê | Porque | Faça |
|---|---|---|
| Perdeu a palavra-passe ou o nó já não se liga à rede | Um nó em configuração e sem endereço após 90 s abre o seu próprio Wi-Fi `ARMOR-SETUP-xxxxxx` (abra `http://192.168.4.1/`) | Para recomeçar mantenha BOOT 8 segundos nos primeiros 30 segundos após ligar (apaga definições e utilizadores), ou use *Reposição de fábrica* no painel |
| Uma atualização do GitHub é recusada | A versão não traz soma de verificação nem assinatura, não coincidem (`e_no_checksum`, `e_no_signature`, `e_checksum_mismatch`, `e_bad_signature`), não traz imagem para esta placa (`e_no_asset_for_board`) ou é de outro projeto (`e_wrong_firmware`) | O nó não mudou; tente de novo ou deixe um administrador enviar o ficheiro à mão |
| A voz não arranca | `voice.enabled` está desligado, ou `voice.url` é `required` ou `invalid` | Ligue-a no painel e dê um endereço válido; mudar de anfitrião apaga o token |
| O rodapé diz a enviar, depois confirmada ou o motivo | Uma ordem vai a caminho, foi confirmada durante alguns segundos, ou foi recusada e guarda o seu motivo | Leia o motivo; decide o servidor, não o painel |

## 📂 Estrutura do repositório

<p align="center">
  <img src="images/repository-structure.svg" alt="Estrutura do repositório de ARMOR-NODE-HMI" width="100%">
</p>

<details>
<summary>Versão em texto da árvore</summary>

```text
ARMOR-NODE-HMI/
├── main/                                o firmware: um componente ESP-IDF
├── core/                                a lógica sem ESP-IDF, testada num computador
├── panel/                               o painel web embebido no firmware
├── tests/                               testes de anfitrião e verificações Python
├── docs/                                9 guias: desenho, voz, tipos de letra, hardware
├── tools/                               ferramentas de compilação, empacotamento e verificação
├── .github/                             fluxo de CI e modelos de issues e pull requests
├── images/                              faixa, ícone e as duas imagens deste README
├── build.sh build.bat                   compilar pelo fluxo de projeto partilhado
├── test.sh test.bat                     executar os testes pelo fluxo partilhado
├── run.sh run.bat                       arrancar pelo fluxo de projeto partilhado
├── armor.project.json                   manifesto: versão, função, maturidade, comando de build
├── CHANGELOG.md                         o que mudou em cada versão
├── SECURITY.md SUPPORT.md LICENSE       política de segurança, apoio e licença
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   como contribuir e o código de conduta
├── CMakeLists.txt                       o projeto ESP-IDF
├── partitions.csv                       a tabela da flash
├── sdkconfig.defaults                   as definições do firmware
├── sdkconfig.board.lcd7box              as definições de uma placa
├── secrets/                             exemplo do ficheiro do nó (os verdadeiros não são versionados)
├── dependencies.lock                    as versões fixadas dos componentes ESP-IDF
├── build-test.sh build-test.bat         executar os testes pelo fluxo partilhado
└── .gitattributes .gitignore            definições do Git
```

</details>

## 🗺️ Roteiro e limites conhecidos

**Limites conhecidos hoje**

* O firmware foi compilado uma vez (ESP-IDF 5.5.5), mas a compilação não é verificada pela integração contínua e nunca funcionou numa placa.
* A troca com o servidor e com o serviço de voz ARMOR-VOICE-AI é testada contra simulações; é premir para falar, sem palavra de ativação.
* A página web correu contra uma simulação num navegador real; o ecrã, o toque e o som foram escritos a partir dos exemplos da Waveshare e esperam a primeira ligação.
* O firmware não funcionou na placa: nada funcionou ainda em equipamentos reais.
* O código de configuração, o limite de acessos, o PIN Bluetooth, o tratamento da coação e as atualizações assinadas estão testados no anfitrião; não houve revisão independente.
* As verificações de hash e de assinatura passam no anfitrião; uma atualização pelo ar nunca foi executada numa placa.
* As mensagens são validadas contra os esquemas partilhados do ARMOR-COMMON; a ligação ao servidor e ao serviço de voz não foi experimentada a partir de uma placa.

**A seguir**

* Fazer a primeira compilação completa e o primeiro arranque na placa, e depois o serviço de voz no Jetson, uma palavra de ativação e a vista de câmaras no ecrã.

## 🌍 Idiomas

O README e os textos existem em inglês, espanhol, francês, italiano, alemão, chinês, japonês e português europeu.

## 🔗 Projetos relacionados

O **A.R.M.O.R.** (Autonomous Radar & Multimodal Observation Range) é um sistema de segurança perimetral feito de repositórios independentes. Cada um tem a sua própria versão, os seus próprios testes e o seu próprio README; esta é a família:

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - Contratos de mensagens, validadores, vetores de conformidade e tipos gerados
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - Firmware de nó de campo para ESP32-S3 com três radares e o seu próprio painel web
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - Protocolos de inversores e baterias solares e as mensagens de um nó porta de ligação
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - Nó elétrico: contadores, a mensagem das leituras da rede e as regras de comutação
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - Nó e painel de alarme: zonas, armar, atrasos, sirene e PIN, com o servidor ou sem ele
* **ARMOR-NODE-HMI** (este repositório) - Painel tátil: o estado do sistema num ecrã de parede, armar e reconhecer, e a casa do assistente de voz
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - A rede local: os seus dispositivos, a internet e o que muda
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - Coordenador central: telemetria, alarmes, dispositivos, leituras solares e câmaras
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Consola web: câmaras, radar, alarmes, energia solar e o desenhador 2D/3D do local
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - Cliente de operador Android com radar 2D/3D em direto
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - Cliente de operador para iPhone
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - Política de inferência visual que explica as suas decisões e nunca atua
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Zonas de câmara, máscaras e regras de deteção para o Jetson Orin NX
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - Intenções de voz offline com uma confirmação que não pode ser falsificada
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - Caixas, eletrónica e a matriz de aceitação em bancada
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - Implantação, a bancada de testes do CM5, cópias de segurança e TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - Simulador de telemetria offline com falhas repetíveis
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - Deteta, instala e atualiza os repositórios do próprio ecossistema
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - Arquitetura, base de segurança e a matriz de capacidades

## 📚 Documentação

* [Desenho](docs/DESIGN.md) e [hardware](docs/HARDWARE.md): o que o painel mostra e a placa
* [Voz](docs/VOICE.md) e [tipos de letra](docs/FONTS.md)
* [Firmware do nó](docs/NODE_FIRMWARE.md) e [ações de compilação](docs/FIRMWARE_BUILD_ACTIONS.md)
* [Testes de bancada](docs/BENCH_TESTS.md), [código de configuração](docs/SETUP_CODE.md) e [provisionamento Bluetooth](docs/BLE_PROVISIONING.md)
* [Matriz de capacidades: o que está demonstrado e o que não está](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [Catálogo de projetos: versões e como os repositórios dependem uns dos outros](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [Registo de alterações deste repositório](CHANGELOG.md)

## 🤝 Contribuir e comunidade

* Antes de um pull request, execute `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py`: tem de passar. Consulte [CONTRIBUTING.md](CONTRIBUTING.md) e o [código de conduta](CODE_OF_CONDUCT.md).
* O README existe em oito idiomas e é gerado a partir das fontes do ARMOR-DOCS: uma alteração ao texto faz-se lá.
* Uma alteração a uma mensagem entre componentes começa no ARMOR-COMMON.
* Não descreva como a funcionar numa placa aquilo que só foi compilado.
* Comunique uma vulnerabilidade em privado: [SECURITY.md](SECURITY.md). Perguntas, ideias e relatórios: electrohobby3d@gmail.com ([SUPPORT.md](SUPPORT.md)).

## 👤 AUTOR

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 LICENÇA

GPL-3.0-or-later - consulte [LICENSE](LICENSE).
