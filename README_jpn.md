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
  🇯🇵 <b>日本語</b> |
  <a href="README_por.md">🇵🇹 Português</a>
</p>

### 家のタッチパネル：壁に掛ける7インチ画面でシステムの状態を表示し、警戒・解除とアラームの確認ができ、将来の音声アシスタント用にマイクとスピーカーを備えています（ESP32-S3-Touch-LCD-7C-BOX 上のファミリーのノードで、設定用の専用Webページ付き）

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

**正直さのチェック - 今日動いているもの:** ボードに依存しない部分は実在し、コンピューター上でテストされています（設定、パネルがサーバーについて知っていることとそれが古くなる時点、決して表示されない強要警報、音声会話と音声サービスの長い応答、ボードのピン、8言語の画面テキストとフォントのグリフ、リリースと署名の検査：18つのテストプログラムで 93,122 項目）。Webページも同様です（実ブラウザで代替機に対して実行）。**ファームウェアは一度ビルドされましたが（ESP-IDF 5.5.5、継続的インテグレーションでは確認していません）、ボード上で動作したことはありません。**画面、タッチ、音、サーバーとの接続は、このボード系列向けの Waveshare のサンプルをもとに書かれており、初めての電源投入が必要です。音声サービス（ARMOR-VOICE-AI）はこのパネルの要求に応答します。ウェイクワードはなく、タッチして話します。

---

## 📋 ひと目で

| | |
|---|---|
| バージョン | `0.4.0` |
| 成熟度 | 機能する |
| 役割 | ARMOR-SERVER 用の壁掛けタッチパネル（ファミリー：Clients、親：ARMOR-SERVER） |
| 技術 | C++17, ESP-IDF 5.5.5, LVGL, ESP32-S3 |
| テスト | 93122 件の検査（ホスト用プログラム 19 本と Python 検査 6 個、`ctest`） |
| 基板 | Waveshare ESP32-S3-Touch-LCD-7C-BOX (800x480, GT911) |
| 動作環境 | Waveshare ESP32-S3-Touch-LCD-7C-BOX、ホストテストは任意のコンピューター |
| 実機での確認 | まだありません：ファームウェアはビルドできます（ESP-IDF 5.5.5、CI では未確認）が、ボード上では動作していません |
| ライセンス | GPL-3.0-or-later |

**移動先：** [概要](#-概要) · [アーキテクチャ](#-アーキテクチャ) · [セキュリティ](#-セキュリティと安全) · [インターフェース](#-インターフェース) · [設定](#-設定) · [クイックスタート](#-クイックスタート) · [テスト](#-テスト) · [トラブルシューティング](#-トラブルシューティング) · [構成](#-リポジトリの構成) · [関連](#-関連プロジェクト)

## 🚦 プロジェクトの状況

各行はこのリポジトリの一部分で、信号は今日どこまで進んでいるかを示します。計画ではなく、リポジトリが示せるもの（テスト、ビルド、実行）に基づいて書かれています。

🟢 完了・検証済み · 🟡 動作する（既知の制限あり） · 🟠 一部のみ、または未検証 · 🔴 未実装または動作しない

| 領域 | 状態 | ここでの意味 |
|---|---|---|
| ソフトウェア | 🟠 | ファームウェアは一度ビルドしました（ESP-IDF 5.5.5）が、そのビルドは継続的インテグレーションでは確認されておらず、ボード上でも動かしていません。 |
| テスト | 🟢 | 19 のホストプログラムで 93,122 件のチェックが通ります（`ctest`）：設定、パネルがサーバーについて知っていることとその古さ、決して表示しない脅迫警報、音声のやり取り、ピン、8 言語の画面文言とフォントのグリフ。 |
| プロトコルとデータ | 🟡 | サーバーおよび音声サービス ARMOR-VOICE-AI とのやり取りは模擬に対してテストしています。プッシュトゥトークで、ウェイクワードはありません。 |
| インターフェース | 🟠 | Web ページは実ブラウザーで模擬に対して動かしました。画面、タッチ、音声は Waveshare のサンプルをもとに書いたもので、初回の電源投入を待っています。 |
| ハードウェア | 🔴 | ファームウェアはボード上で動かしていません。実機ではまだ何も動かしていません。 |
| セキュリティ | 🟡 | 設定コード、ログイン制限、Bluetooth の PIN、脅迫時の扱い、署名付きアップデートはホストでテスト済みです。独立したレビューはまだありません。 |
| アップデート | 🟠 | ハッシュと署名の確認はホストで通ります。無線アップデートはボード上で一度も実行していません。 |
| 連携 | 🟡 | メッセージは ARMOR-COMMON の共有スキーマに対して検証されています。サーバーおよび音声サービスとの接続は、ボードからはまだ試していません。 |
| ドキュメント | 🟢 | 8 言語、ファームウェアガイド、配線メモ、変更履歴は最新です。 |

## 🎯 概要

* **Android アプリと同じく ARMOR-SERVER のクライアント：** 管理者がこのパネル用に作成したユーザーでサインインし、数秒ごとに `GET /api/v1/panel/summary`（数百バイト：モード、オンラインのノード、人の対応が必要なアラーム）を取得し、人がボタンに触れて確認すると警戒・解除・確認を行います。サーバーが応答しなくなると、画面は表示が古いことを伝え、ボタンは無効になります。サーバーのライブチャンネル（WebSocket）を受信して変化をすぐに表示し、チャンネルが無い・無音・無効のときは従来どおり問い合わせます。
* **画面（800x480、LVGL）：** 誰も見ていない最も重大なアラームの色でモードを表示し、確認の質問付きの大きな警戒/解除ボタン、オンラインのノード、アラーム（未確認を先頭に）、夜間の減光、無操作でのスリープ、新しいアラームでの音と点灯、アクセント付きの言語と、使用する文字だけのために生成したフォントで描く日本語と中国語を含む7言語に対応します。
* **他と同じノード：** ARMOR-NODE-RADAR、ARMOR-NODE-SOLAR、ARMOR-NODE-ELECTRICAL と同じ設定ストア、Wi-Fi、8言語のWebページ、スマートフォンからのBluetooth設定、ロールバック付きファームウェア更新、HTTPS を備え（`ARMOR-COMMON/firmware_base` を共有）、サーバー・画面・サウンド・音声のページが加わります。MQTT で Studio に存在を知らせ、Studio がそのページ付きで一覧表示します。
* **音声アシスタント（プッシュトゥトーク）：** タッチでマイクが開き、音声は設定したアドレスの音声サービスにのみ送られ（既定ではオフ）、返答が表示・読み上げされます。手順の順序、時間制限、タッチの動作はテスト済みのステートマシンで、パネルが自らコマンドを実行することはありません（[契約](docs/VOICE.md)）。録音はバイナリ要求で生のバイト列として送られ、音声の返答もバイト列で戻ります。受け付けないサーバーには従来の要求を送ります。
* **ボード：** Waveshare ESP32-S3-Touch-LCD-7C-BOX（フラッシュ32 MB、PSRAM 16 MB、GT911タッチ付き7インチRGB画面、マイク4個、スピーカー）。ピンはWaveshareのサンプル由来で、2つの機能が同じGPIOを共有しないことをテストで証明しています（[ボード](docs/HARDWARE.md)）。
* **未対応：** ファームウェアの最初のコンパイルと電源投入、ウェイクワード、Jetson 上の音声サービス、画面上のカメラ表示。
* **署名付きの無線アップデート：**ノードが GitHub からインストールするのは、SHA-256 が一致し、管理者の有効な Ed25519 署名を持つイメージだけです（公開鍵はファームウェア内、秘密鍵は管理者の手元から出ません）。管理者がパネルで手動アップロードしたファイルは引き続き受け付けます。パネルのページとメッセージは 8 言語です。
* **PIN 付きの Bluetooth ペアリング：**6 桁の PIN は設定コードから導出され、USB コンソールに表示され、管理者にはパネルにも表示されます。スマートフォンはこれを入力して初めてノードの設定を書き込めます。
* **パネルを使えるのは家だけ：**パネルは既定でプライベートネットワーク（または所有者が指定した VPN などのネットワーク）にだけ応答し、保存する本人を締め出す一覧は拒否します。誤ったパスワードは人ごと・アドレスごとに数え、サーバーのプロキシの背後でも動くので、ノードを公開せずに家の外から使えます。
* **何が起きたかを伝えます：** 画面はサーバーが発する全てのアラームコード（ネットワーク、太陽光、電気、警報ゾーンなど）の文言を8言語で持ち、サーバーに壁掛けパネルであることを伝えるため、レーダーとして数えられず、パネルとして一覧されます。
* **ブローカーとの暗号化通信(TLS、任意):** 初期状態ではオフで、ノードはこれまでどおり暗号化なしの MQTT で通信します。ブローカーページのスイッチと、設備独自の認証局の証明書(PEM、最大 2 KB)の入力欄により、`mqtts://` で接続し、その認証局だけを信頼し、到達したアドレス向けに発行された証明書かを確認します。PEM 証明書 1 つでないものは拒否されます。ホストテストで検証済みで、ハンドシェイクはまだ基板では実行していません。
* **状態報告:** ノードは約 1 分ごとに、稼働時間と直前の再起動の理由、空きメモリ、回線と Wi-Fi 信号、ブローカーへの接続回数、最後に記録したエラーを公開します（`armor/health/<ノード>`、ARMOR-COMMON の `node_health` 契約）。作成するコードは全ノードで共有され、コンピューター上でテストされていますが、ボード上ではまだ動作していません。
* **サーバーからの制御：** Studio はノードをきれいに再起動し、最大 1 時間まで一時停止し（自動的に再開）、作業を待機にし（期限なし、再起動後も維持、ノードは接続したまま応答を続けます）、自分を表示させ、診断のスナップショットを求め、メンテナンス中として印を付けることができます（`armor/control/<ノード>/...`、ARMOR-COMMON の `node_control` 契約）。命令はすべて、その操作専用の使い捨てトークンによる 2 段階で、ブローカーに書き込めるのはサーバーだけです。工場出荷時リセットも消去もディープスリープもありません。一時停止中または待機中のパネルは、その旨を示す専用画面を表示してタッチを受け付けません。識別を求められると、画面上部のバーが点滅します（音は鳴らしません）。 このコードは全ノードで共有され、コンピューター上でテスト済みですが、実機ではまだ動作していません。
* **ケーブルなしの診断：** 正常でない起動（クラッシュ、ウォッチドッグ、電源の電圧低下、メモリ確保の失敗）の後、ヘルスレポートが10分間その原因を伝えます。クラッシュするとコアダンプが残り、パネルから確認、ダウンロード、削除できます。保存された設定はファームウェアのインストールごとにコピーされ、更新で壊れた場合は復元されます。管理者はサーバー経由で、使い捨てトークンを使って、ノードに診断パッケージまたはログを要求できます。
* **繰り返すサイレンと順序のある音:** すべての音は優先度付きキュー(サイレン、警報、音声、確認、通知)を通ります。確認されていない重大な警報がある間はサイレンが繰り返され(`audio.siren_repeat_s` 30、`audio.siren_max_repeats` 10)、警報は最小音量を保ちます(`audio.critical_min_volume` 40)。音声は警報に道を譲ります。
* **バッテリーバックアップ付きの時計:** ノードがまだ実時刻を持たず、チップが信頼できる場合は、起動時にボードの PCF85063A チップから時刻を得ます。時刻の同期や設定のたびにチップも補正されます(`time.rtc_enabled`、既定でオン)。
* **画面に出る命令の状態:** 命令が送信中の間はフッターに送信中と表示され、成功すると数秒間「実行しました」と表示されます。拒否された場合は、これまでどおり理由がそこに残ります。
* **保存された秘密情報の任意の封印:** 既定ではオフです(`security.seal_secrets`、またはサーバーの `nodes.nvs_encryption`)。オンにすると、ノードが保存するパスワード、トークン、鍵、PIN、パスワードのハッシュが ChaCha20-Poly1305 で封印され、それ以外は読めるままです。オン/オフの切り替えは何も失わずに書き直し、途中で止まっても大丈夫です。eFuse は焼かれず、フラッシュも暗号化されません。鍵は同じチップのパーティションにあるため、基板を手にした人に対する保護には**なりません**。このパーティションがない時期にインストールされたノードは、新しいパーティションテーブルで書き込むまで従来のままです。

## 🧩 A.R.M.O.R. での役割

ARMOR-NODE-HMI は壁掛けパネルです。Android アプリと同じく [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER) のクライアントで、7 インチ画面、マイク、スピーカーを備えます。モード、オンラインのノード、人が必要な警報を表示し、警戒・解除（PIN 付き）と警報の確認を行います。音声サービスができれば、音声アシスタント（[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)）はここに住み、他のノードと同じく [ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) から設定・更新します。

| 名前 | 方法 | 用途 |
|---|---|---|
| ARMOR-SERVER | HTTPS `GET /api/v1/panel/summary`, WebSocket `/api/v1/panel/events` | 家の概要（モード、アラーム、ノード、エネルギー）とその変化。命令の扱いはサーバーが決める |
| ARMOR-VOICE-AI | サーバー経由 | 1 ターンの音声が音声サービスに送られ、返答が表示・発話される |
| ARMOR-STUDIO | タッチパネルメニュー | パネルの状態を表示し、その Web パネルを開く |
| ARMOR-COMMON | ファームウェア基盤 | 共通ファームウェア（パネル、ログイン、署名付き更新、Bluetooth セットアップ）はここからコピー |

## 🏛️ アーキテクチャ

<p align="center">
  <img src="images/architecture.svg" alt="ARMOR-NODE-HMI のアーキテクチャ：呼び出し元、内部、依存先" width="100%">
</p>

## 🖼️ スクリーンショット

テストベンチで 1920×1080 で撮影。サンプルデータを使用しており、実際の設備は接続されていません。

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="概要：ネットワーク、ブローカー、ファームウェア、報告内容など、ノードの状態をひと目で確認できます。" width="100%"><br><sub>概要：ネットワーク、ブローカー、ファームウェア、報告内容など、ノードの状態をひと目で確認できます。</sub></td><td width="50%" valign="top"><img src="images/screenshots/screen.png" alt="画面と音：明るさ、夜間の時間帯、音量、通知する警報の種類。" width="100%"><br><sub>画面と音：明るさ、夜間の時間帯、音量、通知する警報の種類。</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/voice.png" alt="音声アシスタント：プッシュトゥトーク（既定ではオフ）と、音声の送信先サービス。" width="100%"><br><sub>音声アシスタント：プッシュトゥトーク（既定ではオフ）と、音声の送信先サービス。</sub></td><td width="50%" valign="top"><img src="images/screenshots/server.png" alt="サーバー：A.R.M.O.R. サーバーのアドレス、パネルのユーザー、問い合わせの間隔。" width="100%"><br><sub>サーバー：A.R.M.O.R. サーバーのアドレス、パネルのユーザー、問い合わせの間隔。</sub></td></tr>
</table>

## 🔒 セキュリティと安全

| 脅威 | 対策 |
|---|---|
| 平文またはコンパイル済みのパスワード | PBKDF2-HMAC-SHA256 10 000 ラウンド。ファームウェアにパスワードはなく（ユーザーのないノードはセットアップに入る）、ユーザーは最大 4 人（管理者または閲覧者）、8〜64 文字、セッションは最大 6、無操作 30 分 |
| 書き込み直後のノードの乗っ取り | セットアップには起動コードが必要です。ビルドのもの（`CONFIG_ARMOR_SETUP_CODE`）か、フラッシュに保存して USB コンソールに表示するランダムなもの。フリートシークレットがあれば `HMAC-SHA256(シークレット, MAC)`。これはセットアップ用 Wi-Fi（WPA2、ブリッジなし）の鍵でもあります |
| インターネットからのパネルへのアクセス | 初期設定では家庭のプライベートネットワーク（10/8、172.16/12、192.168/16、169.254/16、127/8、一部の VPN が使う 100.64/10）にだけ応答します。設定 `web.admin_sources` で他の IPv4 アドレスやネットワークを追加でき（8 件、120 文字）、それ以外は公開の `GET /api/v1/session` を除き拒否します。保存した本人を締め出すリストは拒否されます |
| HTTP で平文のパネル | 443 番ポートで HTTPS。証明書はノード自身が作成（自己署名、フラッシュ保存、比較用に SHA-256 フィンガープリントを表示）。パネルは http、https、または両方を選べ、セッション Cookie は常に HttpOnly と SameSite=Strict（TLS では Secure も） |
| GitHub からの偽ファームウェア | SHA-256 が一致し、メンテナーの鍵による有効な Ed25519 署名があるイメージだけをインストールします（`no_checksum`、`no_signature`、`bad_signature`、`checksum_mismatch`、`wrong_firmware`、`not_newer`、`version_mismatch` は拒否）。管理者の手動アップロードは検証されません（明示した制限） |
| Bluetooth セットアップの乗っ取り | セットアップコードから導出した 6 桁の PIN を USB コンソールと管理者に表示し、設定を書き込む前にスマホで入力させます。セットアップの時間枠は `CONFIG_ARMOR_BLE_SETUP_MINUTES`（初期 10、1〜120） |
| 基板を持つ人による保存済みシークレットの読み取り | 任意（初期はオフ、スイッチ `security.seal_secrets` またはサーバーの `nodes.nvs_encryption`）：パスワード、トークン、鍵、PIN、ハッシュを ChaCha20-Poly1305 で保護。フラッシュ全体の暗号化や eFuse の焼き込みはしません。鍵は同じチップの別パーティションにあるため、基板を手にした人には防御になりません |
| ブローカーとの平文通信 | 任意の TLS（`mqtts://`）。所有者自身の認証局（PEM 2 KB まで）と名前の検証を使い、初期はオフです。ハンドシェイクはホストのテストでしか動いておらず、基板では未実行です |
| 強要アラームが画面に表示される | 表示されません（ホストのテストで検査）。パネルがサーバーについて知っていることは、古くなると期限切れと表示されます |
| サーバーになりすました相手 | サーバーの証明書は固定され（`server_pin`）、要求とライブチャネルの WebSocket の両方に使われます。パネルは管理者が作った専用ユーザー（ロール `panel`、Studio は使えません）でサインインします |
| PIN なしのディスアーム | アームとディスアームは先に確認し、画面で PIN を受け付けます（`pin_entry`）。サーバーも再確認します |
| 音声が想定外の場所へ送られる | 初期はオフ（`voice.enabled` が false）。音声は設定にあるアドレスのサービスにだけ送られ（アドレスは検証され、ホストを変えるとトークンは消えます）、パネルが自分でコマンドを実行することはなく、ウェイクワードもなくプッシュトゥトークです |
| 誰にも聞こえないサイレン | 優先度つきキューは 1 つだけ（サイレン、アラーム、音声、確認、通知）。重大アラームが未確認の間はサイレンを繰り返し（`audio.siren_repeat_s` 30、10〜600、0 = 繰り返しなし、`audio.siren_max_repeats` 10）、重大時の音量は `audio.critical_min_volume`（40 %）を下回りません |

ファームウェアは一度ビルドしただけで、基板では一度も動いていません。画面、タッチ、音、サーバーとの接続は Waveshare のサンプルから書いたもので、初めての電源投入を待っています。脆弱性の報告は [SECURITY.md](SECURITY.md) を参照してください。

## 🔌 インターフェース

| 部分 | 設定内容 |
|---|---|
| ARMOR-SERVER | ライブチャネルが落ちている間だけ `server.poll_s`（3 秒、1〜60）ごとに `GET /api/v1/panel/summary`、`server.live_events` が true（既定）の間は WebSocket `/api/v1/panel/events`。モード、アラーム、ノード、エネルギーが変わると概要が届きます。パネル専用ユーザーのロールは `panel` |
| 音声 | 1 ターンの音声はサーバー経由で ARMOR-VOICE-AI に送られます（`/api/v1/voice/turn`、multipart ボディは任意）。`core/voice_session.hpp` が状態機械（手順の順序、時間制限、タッチの動作）です。[docs/VOICE.md](docs/VOICE.md) を参照 |
| ハードウェア | Waveshare ESP32-S3-Touch-LCD-7C-BOX：7 インチ RGB 画面 800x480、GT911 タッチ、マイク 4 個、スピーカー、RTC PCF85063A（`time.rtc_enabled`、既定は true）、フラッシュ 32 MB、PSRAM 16 MB。どの機能も GPIO を共有しないことをテストで確認 |

## ⚙️ 設定

| 変数 | 既定値 | 設定内容 |
|---|---|---|
| `server.poll_s`, `server.live_events` | 3 (1-60), true | 概要の問い合わせ周期と WebSocket のライブチャネル |
| `display.sleep_s` | 300 (0-3600) | 画面がスリープするまでの秒数（0 = なし）。明るさと夜間の明るさも設定です |
| `audio.critical_min_volume`, `audio.siren_repeat_s`, `audio.siren_max_repeats` | 40 (0-100), 30, 10 | 重大な音の最小音量、サイレンの繰り返し周期（0 または 10〜600 秒）、最大繰り返し回数 |
| `audio.tone_by_source` | 重大度に従う | 警報の発生元ごと(デバイス、ソーラー、電気、ネットワーク、中央、その他)の音:重大度どおり、通知、警報、サイレン、または無音、あるいは `spoken`(アラームを音声で読み上げる)、`custom`(その発生元用にアップロードした音)。後の 2 つはサーバーが作成し、欠けずに届いた場合だけ再生され、届かなければ通常の音になります。選んだ音は最低重大度を下回っても鳴ります |
| `audio.clip_max_bytes` | 200000 | 音声で読み上げるアラームや独自の音のためにサーバーから受け取る音声の最大量(16 kHz・16 ビット・モノラルのバイト数、3200〜262144)。これより長い応答は再生されません |
| `voice.enabled`, `voice.url`, `voice.token` | false, "", - | 音声サービス：初期はオフ。アドレスは 160 文字以内で検証され（満たさないと `required` か `invalid`）、ホストが変わるとトークンは消えます |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | すべてのノードに共通する設定：名前（空なら `armor-` と MAC の下位バイト）、セットアップコード（空ならランダム）、フリートシークレット（16 文字以上、リポジトリには置かない）、最初のブローカー設定、ハートビート（2〜300 秒）、Bluetooth セットアップの分数（1〜120）、BOOT によるリセット（最初の 30 秒に 8 秒） |

## 🚀 クイックスタート

**必要なもの:** ホストテスト用の C++17 コンパイラ、イメージのビルド用の ESP-IDF 5.5.5 コンテナ、Waveshare ESP32-S3-Touch-LCD-7C-BOX。

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel of the node without a board
tools/build_node.sh                                   # the image in the ESP-IDF container
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

[設計](docs/DESIGN.md)、[ボード](docs/HARDWARE.md)、[音声アシスタント](docs/VOICE.md)、[ファームウェアガイド](docs/NODE_FIRMWARE.md)、[フォント](docs/FONTS.md)を参照してください。

GitHub Actions で手動でファームウェアをビルドするには、[GitHub Actions でのビルド](docs/FIRMWARE_BUILD_ACTIONS.md)を参照してください。

## 🧪 テスト

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

ホスト用テストプログラム 18 個と Python の検査 6 個を `ctest` で実行します。ホストのプログラムが数えた検査は 93122 件で、`-Werror` でビルドします。

Python の検査：8 言語のパネル文言、タスク、保存パス、ロック下の Bluetooth セッション、パネルが呼ぶルート（ノードによっては HTTP コードやフォントのグリフも）

ホストテストには任意の C++17 コンパイラ（Linux、WSL、MSYS2）が必要です。Web パネルは代役ノード（`tools/panel_mock.mjs`、`tools/panel_browser_test.mjs`）を相手に実際のブラウザでも検証します。

`check_font_glyphs.py` が、フォントに 8 言語のすべてのグリフがあることを検査します（98 行）。`test_clock` は `ok` だけを出力します。

## 🏗️ ビルドとリリース

`tools/build_node.sh` が ESP-IDF 5.5.5 コンテナでイメージをビルドし、リリースにはイメージ、その SHA-256 と Ed25519 署名が基板ごとに付きます。イメージは初回のみ USB-C で書き込み、その後はパネルから更新します。

## 🩺 トラブルシューティング

| 症状 | 原因 | 対処 |
|---|---|---|
| パスワードを忘れた、またはノードがネットワークに参加しない | セットアップ中でアドレスがないノードは 90 秒後に独自の Wi-Fi `ARMOR-SETUP-xxxxxx` を開きます（`http://192.168.4.1/` を開く） | 最初からやり直すには、電源投入後 30 秒以内に BOOT を 8 秒押し続ける（設定とユーザーを消去）か、パネルの*工場出荷状態に戻す*を使います |
| GitHub からの更新が拒否される | リリースにチェックサムや署名がない、一致しない（`e_no_checksum`、`e_no_signature`、`e_checksum_mismatch`、`e_bad_signature`）、この基板用のイメージがない（`e_no_asset_for_board`）、または別プロジェクトのもの（`e_wrong_firmware`） | ノードは変更されていません。再試行するか、管理者が手動でファイルをアップロード |
| 音声が始まらない | `voice.enabled` がオフ、または `voice.url` が `required` か `invalid` | パネルでオンにして有効なアドレスを入れる（ホストを変えるとトークンは消える） |
| フッターが「送信中」のあと「確認済み」か理由を表示する | 命令が送信中、数秒間確認済み、または拒否されて理由が残っている状態 | 理由を読む（決めるのはサーバーでパネルではない） |

## 📂 リポジトリの構成

<p align="center">
  <img src="images/repository-structure.svg" alt="ARMOR-NODE-HMI のリポジトリ構造" width="100%">
</p>

<details>
<summary>ツリーのテキスト版</summary>

```text
ARMOR-NODE-HMI/
├── main/                                ファームウェア（ESP-IDF コンポーネント）
├── core/                                ESP-IDF なしのロジック（コンピューターでテスト）
├── panel/                               ファームウェアに埋め込まれた Web パネル
├── tests/                               ホストテストと Python 検査
├── docs/                                9 つのガイド：設計、音声、フォント、ハードウェア
├── tools/                               ビルド・パッケージ・検査のツール
├── .github/                             CI ワークフローと Issue・Pull Request のテンプレート
├── images/                              バナー、アイコン、この README の 2 つの図
├── build.sh build.bat                   共通のプロジェクトワークフローでビルド
├── test.sh test.bat                     共通のワークフローでテストを実行
├── run.sh run.bat                       共通のプロジェクトワークフローで起動
├── armor.project.json                   マニフェスト：バージョン、役割、成熟度、ビルドコマンド
├── CHANGELOG.md                         各バージョンの変更点
├── SECURITY.md SUPPORT.md LICENSE       セキュリティポリシー、サポート、ライセンス
├── CONTRIBUTING.md CODE_OF_CONDUCT.md   貢献方法と行動規範
├── CMakeLists.txt                       ESP-IDF プロジェクト
├── partitions.csv                       フラッシュのテーブル
├── sdkconfig.defaults                   ファームウェアの設定
├── sdkconfig.board.lcd7box              基板ごとの設定
├── secrets/                             ノードファイルの例（本物はバージョン管理しません）
├── dependencies.lock                    ESP-IDF コンポーネントの固定バージョン
├── build-test.sh build-test.bat         共通のワークフローでテストを実行
└── .gitattributes .gitignore            Git の設定
```

</details>

## 🗺️ ロードマップと既知の制限

**現時点の既知の制限**

* ファームウェアは一度ビルドしました（ESP-IDF 5.5.5）が、そのビルドは継続的インテグレーションでは確認されておらず、ボード上でも動かしていません。
* サーバーおよび音声サービス ARMOR-VOICE-AI とのやり取りは模擬に対してテストしています。プッシュトゥトークで、ウェイクワードはありません。
* Web ページは実ブラウザーで模擬に対して動かしました。画面、タッチ、音声は Waveshare のサンプルをもとに書いたもので、初回の電源投入を待っています。
* ファームウェアはボード上で動かしていません。実機ではまだ何も動かしていません。
* 設定コード、ログイン制限、Bluetooth の PIN、脅迫時の扱い、署名付きアップデートはホストでテスト済みです。独立したレビューはまだありません。
* ハッシュと署名の確認はホストで通ります。無線アップデートはボード上で一度も実行していません。
* メッセージは ARMOR-COMMON の共有スキーマに対して検証されています。サーバーおよび音声サービスとの接続は、ボードからはまだ試していません。

**今後**

* 基板での最初のフルビルドと初めての電源投入を行い、続いて Jetson の音声サービス、ウェイクワード、画面でのカメラ表示に進む。

## 🌍 言語

README とテキストは、英語、スペイン語、フランス語、イタリア語、ドイツ語、中国語、日本語、ヨーロッパポルトガル語で提供されています。

## 🔗 関連プロジェクト

**A.R.M.O.R.**（Autonomous Radar & Multimodal Observation Range）は、独立したリポジトリで構成される境界セキュリティシステムです。各リポジトリは独自のバージョン、テスト、README を持ちます。ファミリーは次のとおりです：

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - メッセージのコントラクト、バリデーター、適合ベクトル、生成された型
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - 3 つのレーダーと独自の Web パネルを備えた ESP32-S3 向けフィールドノードのファームウェア
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - ソーラーインバーターとバッテリーのプロトコル、ゲートウェイノードのメッセージ
* **[ARMOR-NODE-ELECTRICAL](https://github.com/JuanenRac/ARMOR-NODE-ELECTRICAL)** - 電気ノード：メーター、系統の読み取り値のメッセージ、切り替えのルール
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - 警報ノードとパネル：ゾーン、警戒、遅延、サイレン、PIN。サーバーありでもなしでも動作
* **ARMOR-NODE-HMI**（このリポジトリ） - タッチパネル：壁面画面でのシステムの状態、警戒と確認応答、そして音声アシスタントの置き場
* **[ARMOR-NETWORK](https://github.com/JuanenRac/ARMOR-NETWORK)** - ローカルネットワーク：そのデバイス、インターネット、そして変化
* **[ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER)** - 中央コーディネーター：テレメトリ、警報、デバイス、ソーラーの読み取り値、カメラ
* **[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO)** - Web コンソール：カメラ、レーダー、警報、太陽光エネルギー、2D/3D のサイトデザイナー
* **[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL)** - ライブの 2D/3D レーダーを備えた Android オペレータークライアント
* **[ARMOR-IOS-CONTROL](https://github.com/JuanenRac/ARMOR-IOS-CONTROL)** - iPhone オペレータークライアント
* **[ARMOR-SERVER-AI](https://github.com/JuanenRac/ARMOR-SERVER-AI)** - 判断を説明し、決して作動させない視覚推論ポリシー
* **[ARMOR-VISION](https://github.com/JuanenRac/ARMOR-VISION)** - Jetson Orin NX 向けのカメラゾーン、マスク、検出ルール
* **[ARMOR-VOICE-AI](https://github.com/JuanenRac/ARMOR-VOICE-AI)** - 偽造できない確認つきのオフライン音声インテント
* **[ARMOR-HARDWARE](https://github.com/JuanenRac/ARMOR-HARDWARE)** - 筐体、電子回路、ベンチ受け入れマトリクス
* **[ARMOR-DEVOPS](https://github.com/JuanenRac/ARMOR-DEVOPS)** - デプロイ、CM5 テストベンチ、バックアップ、TLS
* **[ARMOR-SIMULATOR](https://github.com/JuanenRac/ARMOR-SIMULATOR)** - 再現可能な障害を持つオフラインのテレメトリシミュレーター
* **[ARMOR-UPDATER](https://github.com/JuanenRac/ARMOR-UPDATER)** - エコシステム自身のリポジトリを検出、インストール、更新
* **[ARMOR-DOCS](https://github.com/JuanenRac/ARMOR-DOCS)** - アーキテクチャ、セキュリティのベースライン、機能マトリクス

## 📚 ドキュメント

* [設計](docs/DESIGN.md)と[ハードウェア](docs/HARDWARE.md)：パネルの表示内容と基板
* [音声](docs/VOICE.md)と[フォント](docs/FONTS.md)
* [ノードのファームウェア](docs/NODE_FIRMWARE.md)と[ビルド手順](docs/FIRMWARE_BUILD_ACTIONS.md)
* [ベンチテスト](docs/BENCH_TESTS.md)、[セットアップコード](docs/SETUP_CODE.md)、[Bluetooth プロビジョニング](docs/BLE_PROVISIONING.md)
* [機能マトリクス：何が証明され、何が証明されていないか](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/CAPABILITY_MATRIX.md)
* [プロジェクトカタログ：バージョンと、リポジトリ同士の依存関係](https://github.com/JuanenRac/ARMOR-DOCS/blob/main/docs/PROJECT_CATALOG.md)
* [このリポジトリの変更履歴](CHANGELOG.md)

## 🤝 コントリビュートとコミュニティ

* Pull Request の前に `ctest --test-dir build/host` and `python -I tools/armor_ci_validate.py` を実行してください。通る必要があります。[CONTRIBUTING.md](CONTRIBUTING.md) と[行動規範](CODE_OF_CONDUCT.md)を参照してください。
* README は 8 言語で提供され、ARMOR-DOCS のソースから生成されます。文面の変更はそちらで行います。
* コンポーネント間のメッセージの変更は ARMOR-COMMON から始めます。
* ビルドしただけのものを、ボード上で動いているかのように書かないでください。
* 脆弱性は非公開で報告してください：[SECURITY.md](SECURITY.md)。質問、アイデア、報告：electrohobby3d@gmail.com（[SUPPORT.md](SUPPORT.md)）。

## 👤 作者

**JuanenRac (Electro Hobby 3D)** · electrohobby3d@gmail.com

## 📜 ライセンス

GPL-3.0-or-later - [LICENSE](LICENSE) を参照してください。
