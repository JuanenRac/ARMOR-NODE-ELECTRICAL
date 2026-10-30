<p align="center">
  <img src="images/ARMOR_BANNER.svg" alt="ARMOR-NODE-ELECTRICAL banner" width="100%">
</p>

# ⚡ ARMOR-NODE-ELECTRICAL

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

### 電気ノード：住宅の AC・DC ネットワークの電圧、電流、電力、エネルギーを読み取り、その開閉のルールを持ちます（PC でテスト済みのコアとビルドできるファームウェア。基板ではまだ動作していません）

<p align="center">
  <img src="https://img.shields.io/badge/License-GPL%203.0--or--later-blue.svg" alt="GPL 3.0 or later">
  <img src="https://img.shields.io/badge/Language-C%2B%2B17-00599c.svg" alt="Language">
  <img src="https://img.shields.io/badge/Meters-PZEM--004T%20%2F%20017-ffb020.svg" alt="Meters">
  <img src="https://img.shields.io/badge/Checks-376%2C195-2ea44f.svg" alt="Checks">
  <img src="https://img.shields.io/badge/Tests-382264%20checks-2ea44f.svg" alt="Tests">
  <img src="https://img.shields.io/badge/Version-0.5.1-0f6cbd.svg" alt="Version">
  <img src="https://img.shields.io/badge/Maturity-functional-00E5FF.svg" alt="Maturity">
</p>

---

**正直さのチェック - 今日動いているもの:** コア（PZEM-004T v3 と PZEM-017 メーターのフレーム、それらに問い合わせるバス、ノードのメッセージ、アナログ入力、切り替えのルール：19 のテストプログラムで 382,264 項目、うち 375,036 項目が切り替えルール）は代替メーターと接触器を使ってコンピューター上でテストされ、生成するメッセージは ARMOR-COMMON に受け付けられます。ノードのファームウェア（設定、シリアルライン、パネルのメーターと測定値のページ、Bluetooth、署名付きアップデート）は ESP-IDF 5.5.5 コンテナーで両方の基板向けにビルドできます。**基板上で動いたことはなく、メーターにも接触器にも何も接続されておらず、ここには何かを切り替えるものはありません。**

---

## 📋 ひと目で

| | |
|---|---|
| バージョン | `0.5.1` |
| 成熟度 | 機能する |
| 役割 | 電気フィールドノード：メーター、入力、開閉（ファミリー：Field Nodes、親：ARMOR-COMMON） |
| 技術 | C++17, ESP-IDF 5.5.5, ESP32-S3 |
| テスト | 382264 件の検査（ホスト用プログラム 19 本と Python 検査 5 個、`ctest`） |
| 機器 | PZEM-004T v3 と PZEM-017 のメーター（最大 16）、アナログ入力、リレー。開閉は初期でオフ |
| 動作環境 | ESP32-S3 基板（Ethernet または Wi-Fi）、ホストテストは任意のコンピューター |
| 実機での確認 | まだありません：基板は未製作で、メーターも接触器も未接続。何も切り替えません |
| ライセンス | GPL-3.0-or-later |

**移動先：** [概要](#-概要) · [アーキテクチャ](#-アーキテクチャ) · [セキュリティ](#-セキュリティと安全) · [インターフェース](#-インターフェース) · [設定](#-設定) · [クイックスタート](#-クイックスタート) · [テスト](#-テスト) · [トラブルシューティング](#-トラブルシューティング) · [構成](#-リポジトリの構成) · [関連](#-関連プロジェクト)

## 🚦 プロジェクトの状況

各行はこのリポジトリの一部分で、信号は今日どこまで進んでいるかを示します。計画ではなく、リポジトリが示せるもの（テスト、ビルド、実行）に基づいて書かれています。

🟢 完了・検証済み · 🟡 動作する（既知の制限あり） · 🟠 一部のみ、または未検証 · 🔴 未実装または動作しない

| 領域 | 状態 | ここでの意味 |
|---|---|---|
| ソフトウェア | 🟢 | ファームウェアは ESP-IDF 5.5.5 コンテナで両方のボード（Wi-Fi とイーサネット）向けにビルドできます。 |
| テスト | 🟢 | 19 のホストプログラムで 382,264 件のチェックが通ります（`ctest`）。そのうち 375,036 件は切り替え規則です。メーターとコンタクターは模擬です。 |
| プロトコルとデータ | 🟡 | PZEM-004T v3 と PZEM-017 のフレームと読み取り値は、文書化されたフレームと記録したサンプルに対して検証済みです。実際のメーターはまだ応答していません。 |
| インターフェース | 🟡 | Web パネルのメーターと読み取り値のページは、ブラウザーで模擬ノードに対して試しており、ボード上では試していません。 |
| ハードウェア | 🔴 | ボードは未製作で、メーターやコンタクターも接続していません。実機ではまだ何も動かしていません。 |
| セキュリティ | 🟡 | 設定コード、誤ったパスワードの回数制限付きログイン、Bluetooth の PIN、署名付きアップデートはホストでテスト済みです。独立したレビューはまだありません。 |
| アップデート | 🟠 | ハッシュと Ed25519 署名の確認、ロールバックの処理はホストで通ります。無線アップデートはボード上で一度も実行していません。 |
| 機器への書き込み | 🟠 | コンタクターの切り替え規則は模擬のコンタクターに対してのみテストしています。実機を切り替えたことはまだありません。 |
| 連携 | 🟠 | ノードのメッセージは ARMOR-COMMON の共有スキーマと適合ベクターに対して検証されています。MQTT クライアントはテストで本物の Mosquitto を相手に動かしていますが、ボードからサーバーへ送ったことはまだありません。 |
| ドキュメント | 🟢 | 8 言語、ファームウェアガイド、配線メモ、変更履歴は最新です。 |

## 🎯 概要

* **電力量計の読み取り：** Peacefair の **PZEM-004T v3**（AC）と **PZEM-017**（DC）の Modbus RTU フレーム。CRC、レジスタを読む要求、応答の解読（電圧、電流、電力、エネルギー、周波数、力率、警報）。作るのは読み取り要求だけで、電力量計に書き込むもの（アドレス、しきい値、エネルギーのリセット）はどこにも作りません。合わない応答や、実際の電力量計が出せない値を含む応答は拒否します。
* **バス：** 複数の電力量計がつながる回線には 1 台ずつタイムアウト付きで問い合わせ、それぞれの最後の正常な読み取りを保持します。応答のない電力量計は 10 秒後に公開されなくなります。
* **メッセージ** `armor/electrical/<ノード>/state`：チャンネル（回路、線、系統入力、DC バス）ごとの項目に、AC/DC、電圧、電流、電力、エネルギー、周波数、力率、ノードが見ている開閉器の状態、警報を入れます。共有契約に含まれ、状態を運び、命令は運びません。
* **開閉のルール：** ハードウェアとは別に、2 つの電源の切り替えを制御するコントローラー。両方を同時に指令せず、開閉の許可、直前のノードの武装、デッドタイム中ずっと両接触器が開と確認されていることを要求し、指令どおりの状態を示さない接触器は確認応答まで残る故障として扱います。**開閉は初期設定でオフで、どのハードウェアも駆動しません。**
* **開閉器へのコマンド（有効化していません）：** 状態メッセージにはノードの開閉器の状態を載せることができ、共有コントラクトにはコマンド（`arm`、続いて 1 回限りのトークン付きの `close_a` または `close_b`、`open`、`acknowledge`）とノードの応答があります。コアは上記のルールでそれらを読み取り応答し、共有ベクターと模擬の接触器でテスト済みです。どのファームウェアイメージにも含まれていないため接触器を動かすことはできず、ARMOR-SERVER は有効化（`ARMOR_ELECTRICAL_SWITCHING=1`）され、かつブローカーの ACL が許可しない限りコマンドを送りません。
* **見える場所：** ARMOR-STUDIO の*電気設計*が住宅のネットワークを描き、各要素にそのチャンネルの計測値を表示します。ARMOR-SERVER が読み取り値、履歴、合計を保持します。
* **ノードのファームウェア**（Wi-Fi の ESP32-S3-WROOM-1 N16R8、またはケーブル接続の Waveshare ESP32-S3-ETH）：設定、最大 16 台の電力量計のための 1 本のシリアル回線、他のノードと同じ Web パネル（セットアップ、ユーザー、Wi-Fi、ブローカー、無線アップデート、HTTPS）に電力量計ページと測定値ページを加えたもの（8 言語）。読み取り専用で、開閉のルールは組み込まれていません。[ファームウェア](docs/NODE_FIRMWARE.md)を参照。
* **スマートフォンから Bluetooth で設定：** レーダーノードと同じチャンネルです。ARMOR アプリがノードを `ARMOR-XXXXXX` として見つけ、パネルのユーザーとセットアップコードを使って、名前、Wi-Fi、アドレス、ブローカー、Bluetooth モードを設定します（[プロトコル](docs/BLE_PROVISIONING.md)）。設定を変えない限り、ノードにユーザーがいない間だけ待ち受けます。無線部分は基板ではまだ動作していません。
* **署名付きの無線アップデート：**ノードが GitHub からインストールするのは、SHA-256 が一致し、管理者の有効な Ed25519 署名を持つイメージだけです（公開鍵はファームウェア内、秘密鍵は管理者の手元から出ません）。管理者がパネルで手動アップロードしたファイルは引き続き受け付けます。パネルのページとメッセージは 8 言語です。
* **PIN 付きの Bluetooth ペアリング：**6 桁の PIN は設定コードから導出され、USB コンソールに表示され、管理者にはパネルにも表示されます。スマートフォンはこれを入力して初めてノードの設定を書き込めます。
* **まだ：** 基板上で動作するファームウェア、ハードウェア、実際の設備での計測、そして開閉。
* **パネルを使えるのは家だけ：**パネルは既定でプライベートネットワーク（または所有者が指定した VPN などのネットワーク）にだけ応答し、保存する本人を締め出す一覧は拒否します。誤ったパスワードは人ごと・アドレスごとに数え、サーバーのプロキシの背後でも動くので、ノードを公開せずに家の外から使えます。
* **推定値はそう示されます：** 電流入力から作られたチャンネルには `power_estimated` が付き、Studio とサーバーはその電力を推定値（電流と公称電圧からの皮相電力）として表示し、それを含む系統値にも印を付けます。
* **ブローカーとの暗号化通信(TLS、任意):** 初期状態ではオフで、ノードはこれまでどおり暗号化なしの MQTT で通信します。ブローカーページのスイッチと、設備独自の認証局の証明書(PEM、最大 2 KB)の入力欄により、`mqtts://` で接続し、その認証局だけを信頼し、到達したアドレス向けに発行された証明書かを確認します。PEM 証明書 1 つでないものは拒否されます。ホストテストで検証済みで、ハンドシェイクはまだ基板では実行していません。
* **状態報告:** ノードは約 1 分ごとに、稼働時間と直前の再起動の理由、空きメモリ、回線と Wi-Fi 信号、ブローカーへの接続回数、最後に記録したエラーを公開します（`armor/health/<ノード>`、ARMOR-COMMON の `node_health` 契約）。作成するコードは全ノードで共有され、コンピューター上でテストされていますが、ボード上ではまだ動作していません。
* **サーバーからの制御：** Studio はノードをきれいに再起動し、最大 1 時間まで一時停止し（自動的に再開）、作業を待機にし（期限なし、再起動後も維持、ノードは接続したまま応答を続けます）、自分を表示させ、診断のスナップショットを求め、メンテナンス中として印を付けることができます（`armor/control/<ノード>/...`、ARMOR-COMMON の `node_control` 契約）。命令はすべて、その操作専用の使い捨てトークンによる 2 段階で、ブローカーに書き込めるのはサーバーだけです。工場出荷時リセットも消去もディープスリープもありません。一時停止中または待機中のノードは、誰が要求しても（パネル、ブローカー、セントラル）すべてのリレーを開いたままにして動かしません。電力量計の読み取りは続けます。 このコードは全ノードで共有され、コンピューター上でテスト済みですが、実機ではまだ動作していません。
* **鮮度を示す測定値：** 各メーターチャンネルには測定値の経過時間とメーターへの失敗したリクエスト数が付くため、サーバーは本当の0 Wと応答しなくなったメーターを区別できます。メータータスクが固まった場合はウォッチドッグがボードを再起動します。
* **ケーブルなしの診断：** 正常でない起動（クラッシュ、ウォッチドッグ、電源の電圧低下、メモリ確保の失敗）の後、ヘルスレポートが10分間その原因を伝えます。クラッシュするとコアダンプが残り、パネルから確認、ダウンロード、削除できます。保存された設定はファームウェアのインストールごとにコピーされ、更新で壊れた場合は復元されます。管理者はサーバー経由で、使い捨てトークンを使って、ノードに診断パッケージまたはログを要求できます。
* **リレーのインターロック：**リレーは、同時に閉じてはならない別のリレーを指定できます（1つの負荷の2系統の電源、モーターの正転と逆転など）。一方が閉じている間にもう一方を閉じる要求は、パネル、ブローカー、セントラルのいずれからでも拒否され、ノードがもう一方を勝手に開くことはありません。コンピューター上でテスト済みで、ボード上ではまだ動作していません。
* **メーター自身の設定の読み取り専用の確認:** 最初の正常な読み取りの後と `bus.check_settings_s` ごと(600 秒、0 = なし)に、測定と同じ巡回で PZEM のホールディングレジスタ(004T v3:電力のしきい値とアドレス、017:電圧のしきい値、アドレス、シャント)を読み、保存されたアドレスが使用中のものと違う場合は警告します。メーターには何も書き込みません。
* **保存された秘密情報の任意の封印:** 既定ではオフです(`security.seal_secrets`、またはサーバーの `nodes.nvs_encryption`)。オンにすると、ノードが保存するパスワード、トークン、鍵、PIN、パスワードのハッシュが ChaCha20-Poly1305 で封印され、それ以外は読めるままです。オン/オフの切り替えは何も失わずに書き直し、途中で止まっても大丈夫です。eFuse は焼かれず、フラッシュも暗号化されません。鍵は同じチップのパーティションにあるため、基板を手にした人に対する保護には**なりません**。このパーティションがない時期にインストールされたノードは、新しいパーティションテーブルで書き込むまで従来のままです。

## 🧩 A.R.M.O.R. での役割

ARMOR-NODE-ELECTRICAL はメーターで家の AC・DC ネットワークを測定し、電源間の切り替えのルールを保持します。測定値を [ARMOR-SERVER](https://github.com/JuanenRac/ARMOR-SERVER) に送ると、サーバーが履歴と日ごとのエネルギーを保持して警報を出し、[ARMOR-STUDIO](https://github.com/JuanenRac/ARMOR-STUDIO) の*電気*メニューと*電気デザイナー*、[ARMOR-ANDROID-CONTROL](https://github.com/JuanenRac/ARMOR-ANDROID-CONTROL) の電気画面が表示します。切り替えは設計済み（待機、1 回限りのトークン、インターロック、両方の接触器の確認）ですが、有効化はされていません。

| 名前 | 方法 | 用途 |
|---|---|---|
| ARMOR-SERVER | MQTT `armor/electrical/<id>/{state,command,result}` | チャンネルを受け取り、切り替えを命令する（開閉が 3 か所で有効なときのみ） |
| ARMOR-STUDIO | 電気メニュー、Electrical Designer | チャンネルとアラームを表示し、ノードのパネルを開く |
| ARMOR-ANDROID-CONTROL | Bluetooth | 現場でノードを設定し、チャンネルを表示 |
| ARMOR-COMMON | 電気のコントラクト | メッセージはそのスキーマで検証し、共通ファームウェアはここからコピー |

## 🏛️ アーキテクチャ

<p align="center">
  <img src="images/architecture.svg" alt="ARMOR-NODE-ELECTRICAL のアーキテクチャ：呼び出し元、内部、依存先" width="100%">
</p>

## 🖼️ スクリーンショット

テストベンチで 1920×1080 で撮影。サンプルデータを使用しており、実際の設備は接続されていません。

<table>
<tr><td width="50%" valign="top"><img src="images/screenshots/overview.png" alt="概要：ネットワーク、ブローカー、ファームウェア、報告内容など、ノードの状態をひと目で確認できます。" width="100%"><br><sub>概要：ネットワーク、ブローカー、ファームウェア、報告内容など、ノードの状態をひと目で確認できます。</sub></td><td width="50%" valign="top"><img src="images/screenshots/meters.png" alt="メーター：ノードが読み取る電力量計とアナログチャネル。名前とスケールを設定できます。" width="100%"><br><sub>メーター：ノードが読み取る電力量計とアナログチャネル。名前とスケールを設定できます。</sub></td></tr>
<tr><td width="50%" valign="top"><img src="images/screenshots/readings.png" alt="測定値：ノードがいま読み取っている値と、各値の経過時間。" width="100%"><br><sub>測定値：ノードがいま読み取っている値と、各値の経過時間。</sub></td><td width="50%" valign="top"><img src="images/screenshots/zigbee.png" alt="Zigbee 無線：コーディネーターモジュールへのシリアルブリッジ、使用を許可するクライアント、ブリッジの状態。" width="100%"><br><sub>Zigbee 無線：コーディネーターモジュールへのシリアルブリッジ、使用を許可するクライアント、ブリッジの状態。</sub></td></tr>
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
| 害を及ぼす遠隔命令 | その操作に結び付いた使い捨てトークンによる 2 段階。ブローカーのそのトピックに書き込めるのはサーバーだけで、一時停止は最長 1 時間。遠隔命令による工場出荷状態への復元、消去、ディープスリープはありません |
| 基板を持つ人による保存済みシークレットの読み取り | 任意（初期はオフ、スイッチ `security.seal_secrets` またはサーバーの `nodes.nvs_encryption`）：パスワード、トークン、鍵、PIN、ハッシュを ChaCha20-Poly1305 で保護。フラッシュ全体の暗号化や eFuse の焼き込みはしません。鍵は同じチップの別パーティションにあるため、基板を手にした人には防御になりません |
| ブローカーとの平文通信 | 任意の TLS（`mqtts://`）。所有者自身の認証局（PEM 2 KB まで）と名前の検証を使い、初期はオフです。ハンドシェイクはホストのテストでしか動いておらず、基板では未実行です |
| 基板上の危険な商用電源や直流 | ノードは PZEM モジュール（密閉型、絶縁されたシリアル線）を読むだけで、基板に商用電源を通しません。開閉する部分は別の段で、ランプと絶縁トランスを使ってベンチで試験します |
| 2 つの電源が同時に閉じる | 切替インターロックは両方を同時に命令しません。開閉が許可され、直前にノードがアームされ、デッドタイムの間ずっと両方の接触器が開と確認されている必要があります（`dead_time_ms` は既定 2000）。命令どおりの状態を示さない接触器は、確認応答するまで残る故障です（`kDidNotOpen`、`kDidNotClose`）。接触器間の機械的インターロックも引き続き必須です |
| 繰り返された命令、または誰も頼んでいない開閉 | `arm` が 64 ビットの使い捨てトークンを返し、`close_a` か `close_b` は `arm_window_ms` 内にそれを付ける必要があります。命令は保持されず、スイッチは最大 4 つ。開閉は、ファームウェアが許可し（`switching_enabled`）、サーバーが `ARMOR_ELECTRICAL_SWITCHING=1` で動き、ブローカーのアクセスリストがそのノードに開いていない限りオフです |
| 故障、フィードバックの喪失、開閉のオフ | 各コントローラーは故障・フィードバック喪失・開閉オフのとき、すべてのコイルを開いた状態で応答します（開が負荷にとって安全かどうかは設置側が判断）。メッセージの状態は命令ではなく補助接点が示すもの。リレーのペアはインターロックされ、ノードが勝手に相手を開くことはありません |
| 古い測定値が実測値として表示される | 応答しないメーターは 10 秒後に発行を止め、各チャンネルは測定値の古さと失敗した要求の数を持ちます。メーターのタスクが固まるとウォッチドッグで基板を再起動。`bus.check_settings_s`（600 秒、0 = なし）ごとにメーターの設定を読み取り（読み取りのみ）、アドレスが変わっていれば報告します |

商用電源（230 V AC）やバッテリーバンクの直流は命を奪い、火災を起こすことがあります。[docs/SAFETY.md](docs/SAFETY.md) は設計が前提とする事柄を述べたもので、設備の設計ではなく、その国の規則や有資格の施工者の代わりにはなりません。現時点では、ここにあるものは何も開閉しません。

## 🔌 インターフェース

| 部分 | 設定内容 |
|---|---|
| MQTT | `armor/electrical/<id>/state`（入力ごとに 1 チャンネル：AC または DC、電圧、電流、電力、エネルギー、周波数、力率、スイッチの状態、アラーム。ARMOR-COMMON のコントラクト）、`armor/electrical/<id>/command` と `/result`、`armor/device/<id>/<リレー>/{state,set}`、`armor/health/<id>`、`armor/control/<id>/...` |
| コマンド | `arm`、`close_a`、`close_b`（トークン付き）、`open`、`acknowledge`（故障を解除） |
| メーター | Peacefair PZEM-004T v3（AC）と PZEM-017（DC）を Modbus RTU で、1 本のシリアル線に最大 16 台。組み立てるのは読み取り要求だけで（アドレスやしきい値を書き込む要求は存在しません）、アナログ入力、接点、リレーはベース基板上、Zigbee ブリッジあり（[docs/ZIGBEE.md](docs/ZIGBEE.md)） |

## ⚙️ 設定

| 変数 | 既定値 | 設定内容 |
|---|---|---|
| `bus.check_settings_s` | 600 | メーター設定の読み取り専用チェックの間隔（秒、0 = なし） |
| `dead_time_ms`, `confirm_timeout_ms` | 2000 | 切替のデッドタイムと、接触器が命令どおりの状態を示すまでの猶予 |
| `switching_enabled` | off | この設定、サーバー（`ARMOR_ELECTRICAL_SWITCHING=1`）、ブローカーのアクセスリストのすべてが許可しない限り開閉はオフ |
| `CONFIG_ARMOR_NODE_ID`, `_SETUP_CODE`, `_FLEET_SECRET`, `_MQTT_*`, `_HEARTBEAT_S`, `_BLE_SETUP_MINUTES`, `_BOOT_FACTORY_RESET` | - | すべてのノードに共通する設定：名前（空なら `armor-` と MAC の下位バイト）、セットアップコード（空ならランダム）、フリートシークレット（16 文字以上、リポジトリには置かない）、最初のブローカー設定、ハートビート（2〜300 秒）、Bluetooth セットアップの分数（1〜120）、BOOT によるリセット（最初の 30 秒に 8 秒） |

## 🚀 クイックスタート

**必要なもの:** ホストテスト用の C++17 コンパイラ、イメージのビルド用の ESP-IDF 5.5.5 コンテナ、ESP32-S3 基板（Ethernet または Wi-Fi）。

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host   # host tests, -Werror
node tools/panel_mock.mjs --user admin:adminpass123   # the panel without a board
tools/build_node.sh generic s3-wifi                  # the image in the ESP-IDF container (or s3-eth)
```

See the [firmware guide](docs/NODE_FIRMWARE.md) and the [Bluetooth channel](docs/BLE_PROVISIONING.md).

[設計](docs/DESIGN.md)、[安全に関する注意](docs/SAFETY.md)、[開閉のルール](docs/SWITCHING.md)、[プロトコル](docs/PROTOCOLS.md)、[メッセージ](docs/ELECTRICAL_MESSAGES.md)を参照してください。

## 🧪 テスト

```bash
cmake -S tests -B build/host && cmake --build build/host && ctest --test-dir build/host
python -I tools/armor_ci_validate.py
```

ホスト用テストプログラム 19 個と Python の検査 5 個を `ctest` で実行します。ホストのプログラムが数えた検査は 382264 件で、`-Werror` でビルドします。

Python の検査：8 言語のパネル文言、タスク、保存パス、ロック下の Bluetooth セッション、パネルが呼ぶルート（ノードによっては HTTP コードやフォントのグリフも）

ホストテストには任意の C++17 コンパイラ（Linux、WSL、MSYS2）が必要です。Web パネルは代役ノード（`tools/panel_mock.mjs`、`tools/panel_browser_test.mjs`）を相手に実際のブラウザでも検証します。

検査の大半は開閉ルールの網羅的な総当たりです（`test_switch_set` と `test_interlock`：375 036）。メーターと接触器は代役です。`test_clock` は `ok` だけを出力します。

## 🏗️ ビルドとリリース

`tools/build_node.sh` が ESP-IDF 5.5.5 コンテナでイメージをビルドし、リリースにはイメージ、その SHA-256 と Ed25519 署名が基板ごとに付きます。イメージは初回のみ USB-C で書き込み、その後はパネルから更新します。

## 🩺 トラブルシューティング

| 症状 | 原因 | 対処 |
|---|---|---|
| パスワードを忘れた、またはノードがネットワークに参加しない | セットアップ中でアドレスがないノードは 90 秒後に独自の Wi-Fi `ARMOR-SETUP-xxxxxx` を開きます（`http://192.168.4.1/` を開く） | 最初からやり直すには、電源投入後 30 秒以内に BOOT を 8 秒押し続ける（設定とユーザーを消去）か、パネルの*工場出荷状態に戻す*を使います |
| GitHub からの更新が拒否される | リリースにチェックサムや署名がない、一致しない（`e_no_checksum`、`e_no_signature`、`e_checksum_mismatch`、`e_bad_signature`）、この基板用のイメージがない（`e_no_asset_for_board`）、または別プロジェクトのもの（`e_wrong_firmware`） | ノードは変更されていません。再試行するか、管理者が手動でファイルをアップロード |
| スイッチが故障を示す | 接触器が命令どおりの開閉を示さなかった（`kDidNotOpen`、`kDidNotClose`）。確認応答するまで残ります | 補助接点の配線を確認してから `acknowledge` を送る |
| コマンドが拒否される | 開閉がオフ（`switching_enabled` が false）、トークンが無効か期限切れ、または直前にノードがアームされていない | もう一度アームして新しいトークンで命令を送る |

## 📂 リポジトリの構成

<p align="center">
  <img src="images/repository-structure.svg" alt="ARMOR-NODE-ELECTRICAL のリポジトリ構造" width="100%">
</p>

<details>
<summary>ツリーのテキスト版</summary>

```text
ARMOR-NODE-ELECTRICAL/
├── main/                                ファームウェア（ESP-IDF コンポーネント）
├── core/                                ESP-IDF なしのロジック（コンピューターでテスト）
├── panel/                               ファームウェアに埋め込まれた Web パネル
├── tests/                               ホストテストと Python 検査
├── docs/                                12 のガイド：安全、開閉、メッセージ、ハードウェア
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
├── sdkconfig.board.s3-eth               基板ごとの設定
├── sdkconfig.board.s3-wifi              基板ごとの設定
├── secrets/                             ノードファイルの例（本物はバージョン管理しません）
├── build-test.sh build-test.bat         共通のワークフローでテストを実行
└── .gitattributes .gitignore            Git の設定
```

</details>

## 🗺️ ロードマップと既知の制限

**現時点の既知の制限**

* PZEM-004T v3 と PZEM-017 のフレームと読み取り値は、文書化されたフレームと記録したサンプルに対して検証済みです。実際のメーターはまだ応答していません。
* Web パネルのメーターと読み取り値のページは、ブラウザーで模擬ノードに対して試しており、ボード上では試していません。
* ボードは未製作で、メーターやコンタクターも接続していません。実機ではまだ何も動かしていません。
* 設定コード、誤ったパスワードの回数制限付きログイン、Bluetooth の PIN、署名付きアップデートはホストでテスト済みです。独立したレビューはまだありません。
* ハッシュと Ed25519 署名の確認、ロールバックの処理はホストで通ります。無線アップデートはボード上で一度も実行していません。
* コンタクターの切り替え規則は模擬のコンタクターに対してのみテストしています。実機を切り替えたことはまだありません。

**今後**

* ランプと絶縁トランスを使ったベンチでメーター、接触器、開閉ルールを試し、次に実際の基板で OTA 更新を試す。

## 🌍 言語

README とテキストは、英語、スペイン語、フランス語、イタリア語、ドイツ語、中国語、日本語、ヨーロッパポルトガル語で提供されています。

## 🔗 関連プロジェクト

**A.R.M.O.R.**（Autonomous Radar & Multimodal Observation Range）は、独立したリポジトリで構成される境界セキュリティシステムです。各リポジトリは独自のバージョン、テスト、README を持ちます。ファミリーは次のとおりです：

* **[ARMOR-COMMON](https://github.com/JuanenRac/ARMOR-COMMON)** - メッセージのコントラクト、バリデーター、適合ベクトル、生成された型
* **[ARMOR-NODE-RADAR](https://github.com/JuanenRac/ARMOR-NODE-RADAR)** - 3 つのレーダーと独自の Web パネルを備えた ESP32-S3 向けフィールドノードのファームウェア
* **[ARMOR-NODE-SOLAR](https://github.com/JuanenRac/ARMOR-NODE-SOLAR)** - ソーラーインバーターとバッテリーのプロトコル、ゲートウェイノードのメッセージ
* **ARMOR-NODE-ELECTRICAL**（このリポジトリ） - 電気ノード：メーター、系統の読み取り値のメッセージ、切り替えのルール
* **[ARMOR-NODE-ALARM](https://github.com/JuanenRac/ARMOR-NODE-ALARM)** - 警報ノードとパネル：ゾーン、警戒、遅延、サイレン、PIN。サーバーありでもなしでも動作
* **[ARMOR-NODE-HMI](https://github.com/JuanenRac/ARMOR-NODE-HMI)** - タッチパネル：壁面画面でのシステムの状態、警戒と確認応答、そして音声アシスタントの置き場
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

* [安全](docs/SAFETY.md)と[開閉](docs/SWITCHING.md)：設計の前提と切替の流れ
* [メッセージ](docs/ELECTRICAL_MESSAGES.md)、[プロトコル](docs/PROTOCOLS.md)、[Zigbee](docs/ZIGBEE.md)
* [設計](docs/DESIGN.md)、[ハードウェア](docs/HARDWARE.md)、[ベース基板](docs/BASE_BOARD.md)、[ノードのファームウェア](docs/NODE_FIRMWARE.md)
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
