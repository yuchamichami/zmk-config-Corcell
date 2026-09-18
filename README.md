# Corcell ZMK ファームウェア

Corcell は、PAW3222 トラックボールと乾電池駆動に対応した ZMK ファームウェアです。
キー配線、matrix transform、physical layout、charlieplex kscan は Corchibi 互換です。

**このブランチ（`dya-studio`）は DYA Studio 対応版です。**
通常版は `main` ブランチで管理します。

初めて使う方・接続で困っている方は **[接続・使い方ガイド](docs/connection-guide.md)** を参照してください。
USB は **右手側** に接続します。左右間の通信には Bluetooth を使います。

## 初めての接続・セットアップ

**DYA Studio で設定を変えたい場合は、まず USB で接続すると手順が少なく済みます。**
USB 接続のために、先に PC と Bluetooth ペアリングする必要はありません。
文字を無線で入力したいだけなら、下の「Bluetooth でキーボードとして使う」へ進んでください。

### キー操作の読み方

「レイヤー 3 ＋ U」は、**レイヤー 3 キーを押したまま、U の位置のキーを 1 回押し、両方を離す**操作です。
レイヤー 3 キーは、初期配列の **右親指の Space と、短押しで A になるキーの間**にあります。
「3」と「U」を文字として入力する操作ではありません。
以下の位置は初期配列基準です。Studio で配列を変更済みの場合は、ご自身の割り当てを確認してください。

### A. USB で DYA Studio を使う（最初はこちら）

**接続改善版の DYA ファームが入っていることが前提です。** `main`・`lenotp`・旧 DYA 版ではこの USB Studio 手順は使えません。
未導入・版が不明な場合は、[ファーム更新手順](https://github.com/yuchamichami/zmk-config-Corcell/blob/dya-studio/docs/connection-guide.md#8-ファーム更新)を先に確認してください。

1. **右手側**をデータ通信対応 USB ケーブルで PC につなぎます。左手も使う場合は、左側にも電池を入れて電源を入れます。
2. **レイヤー 3 ＋ U 位置**を押して、文字入力と Studio 通信の出力先を **USB 優先**にします。
3. PC の **Chrome または Edge** で [DYA Studio](https://studio.dya.cormoran.works/) を開きます。
4. **USB 接続**を選び、Corcell の Studio 用シリアルポートを選択します。候補が 2 つあり一方で応答しない場合は、接続を閉じてもう一方を試してください。
5. 配列などを編集し、画面の **保存**を押します。保存完了後、10 秒以上待ってから電源を切ります。

**この USB 手順では Studio Unlock を押す必要はありません。** USB 経由で文字を打ちながら設定を編集できます。
なお、USB 使用時も **左手から右手への通信は無線**です。

### B. Bluetooth でキーボードとして使う

この手順は通常版と DYA 版のどちらでも使えます。`lenotp` は実験構成での確認用です。

1. USB を外し、左右に電池を入れて電源を入れます。
2. **レイヤー 3 ＋右上端の Backspace 位置**を押して、接続先 0 を選びます。既に別端末を登録している場合は、ガイドを参照して空き接続先を選びます。
3. PC／スマホの **OS の Bluetooth 設定**から **Corcell** を追加します。左右を別々に登録する必要はありません。
4. メモ帳などで、右手と左手のキーが入力できることを確認します。

**文字を打つだけなら、ここで完了です。DYA Studio を開いたり、Studio Unlock を押したりする必要はありません。**
接続改善版で USB 給電を残したまま無線入力したい場合は、**レイヤー 3 ＋ I 位置**で Bluetooth 優先にします。

### C. Bluetooth 経由で DYA Studio も使いたい場合

接続改善版の DYA ファーム向けです。先に B の手順で、編集に使う PC と Bluetooth 接続し、文字入力ができる状態にします。

1. USB は外しておきます。USB 給電を残す場合は、**レイヤー 3 ＋ I 位置**で Bluetooth 優先にします。
2. **レイヤー 3 ＋右親指の短押し A の位置**を押します。これが **Studio Unlock** で、ブラウザから Corcell を見つけられるようにする操作です。
3. 対応ブラウザで [DYA Studio](https://studio.dya.cormoran.works/) を開き、**Bluetooth 接続**から Corcell を選びます。
4. 編集後に **保存**します。

現在の DYA 版は起動時から設定変更のロックを解除しています。Bluetooth で押す Studio Unlock は、主に **ブラウザでの検出を開始するため**です。
PC／ブラウザの対応状況も接続に影響します。検出で迷った場合は A の USB 手順で確認してください。

### 迷ったときの早見表（接続改善版の初期配列）

| やりたいこと | 操作 |
| --- | --- |
| USB で入力・Studio 編集 | 右に USB → レイヤー 3 ＋ U → Studio の USB 接続。Unlock 不要 |
| Bluetooth で文字入力だけ | 接続先選択 → OS で Corcell を登録。Unlock 不要 |
| Bluetooth で Studio 編集 | OS で入力確認 → USB を外す（またはレイヤー 3 ＋ I）→ Studio Unlock → Studio の Bluetooth 接続 |

**レイヤー 3 ＋ U は USB 用です。Bluetooth 接続の前に押す操作ではありません。**
旧 DYA 版や保存済みの独自配列には、新しい出力切替キーがない場合があります。

### 接続・書き込みで困ったら

- `The port is already open` はまず Studio のタブを閉じ、USB を抜き差しして 1 タブだけで再接続します。
- UF2 は ZIP を展開し、右用を右、左用を左へ **1 ファイルずつ**コピーします。Windows の `0x80070022` だけでは書き込みの成否を判断せず、通常起動と更新版を確認してください。
- `settings_reset` は保存済み配列・ペアリングなどを消す最終手段です。通常の接続や更新では不要です。

**詳しい接続・更新・復旧手順： [接続・使い方ガイド](https://github.com/yuchamichami/zmk-config-Corcell/blob/dya-studio/docs/connection-guide.md)**

[Discord 向け案内文](https://github.com/yuchamichami/zmk-config-Corcell/blob/dya-studio/docs/discord-connection-announcement.md) ／ [点検結果と実機確認項目](https://github.com/yuchamichami/zmk-config-Corcell/blob/dya-studio/docs/firmware-audit-2026-09-06.md)


## ブランチの選び方

利用者向けは **`main` と `dya-studio`** です。各ブランチの README にセットアップ手順を記載しています。

| ブランチ | 用途 |
| --- | --- |
| [`main`](https://github.com/yuchamichami/zmk-config-Corcell/tree/main) | 通常版。ブラウザでの設定編集を使わない方向け |
| [`dya-studio`](https://github.com/yuchamichami/zmk-config-Corcell/tree/dya-studio) | DYA Studio 対応版。ブラウザでキーマップやポインタ倍率を調整する方向け |
| [`lenotp`](https://github.com/yuchamichami/zmk-config-Corcell/tree/lenotp) | lenoTP の実験用。通常の利用者向け配布版ではありません |

## セッティングガイド

ボトムケースの開けかたと、チルトスタンドのサポート材の除去は
動画つきの別ページにまとめています。

**→ [セッティングガイド](docs/setup-guide.md)**

## ハードウェア構成

- `corcell_l` は split peripheral です。左側のモジュール入力を右側へ転送します。
- `corcell_r` は split central です。右側のモジュール入力と、左側から転送された入力を扱います。
- キー配線と kscan ピンは Corchibi と同じです。
- 6 ピン FPC スロットには、左右それぞれ 1 つずつ任意の入力モジュールを接続できます。
- デフォルトの FPC モジュールは PAW3222 トラックボールです。
- PAW3222 は `SCLK=P0.10`、`SDIO=P0.09`、`MOTION=P1.12` を使います。
- PAW3222 の NCS はデフォルトで GND 固定です。そのため、ファームウェア側では SPI chip-select GPIO を設定していません。
- PAW3222 で chip-select GPIO 制御が必要になった場合のみ、NCS を `RE_B` 側へジャンパして `&spi0` 配下に `cs-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;` を追加します。
- PAW3222 の CPI はファームウェア側で上書きせず、DYA Studio の runtime input processor
  倍率で調整します。既定値はカーソル `2/5`、スクロール `1/10` です。
- 基板上のロータリーエンコーダーは `RE_A=P0.04`、`RE_B=P0.05` で、デフォルトで有効です。
- FPC エンコーダーモジュールでは、PAW3222 の `NCS` 位置を A 相、`MOTION` 位置を B 相として使います。
- 現行回路では、エンコーダーモジュール使用時に `NCS` を `RE_B` 側へジャンパしてください。このときファームウェアは `A=P0.05`、`B=P1.12` として読みます。
- 乾電池の入力電圧は ADC0 / `P0.02` で読みます。

## FPC モジュールの切り替え

FPC モジュールは Zephyr/ZMK のスニペットで切り替えます。
通常の `build.yaml` では左右に PAW3222、右手に加えて Studio USB 用 snippet を指定します。
生成される UF2 は右手・左手・設定初期化用の 3 つです。

- 右手 PAW3222: `corcell-right-slot1-paw3222`
- 左手 PAW3222: `corcell-left-slot1-paw3222`
- 右手エンコーダー: `corcell-right-slot1-encoder`
- 左手エンコーダー: `corcell-left-slot1-encoder`

たとえば右手スロットをエンコーダーにする場合は、`build.yaml` の
`corcell-right-slot1-paw3222` を `corcell-right-slot1-encoder` に変更します。

ユーザー目線では次の流れです。

1. 左右それぞれ、FPC スロットに取り付けるモジュールを決めます。
2. `build.yaml` の `corcell_r` と `corcell_l` の FPC 用 snippet を選びます。右手側の `studio-rpc-usb-uart` は残します。
3. 変更を push します。
4. GitHub Actions の `Build` が完了したら、Artifacts から UF2 をダウンロードします。
5. `Corcell_R-...uf2` を右手、`Corcell_L-...uf2` を左手に書き込みます。

たとえば右手をエンコーダー、左手を PAW3222 にする場合は次のようにします。

```yaml
include:
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_r
    artifact-name: Corcell_R-dya-studio-xiao_ble_zmk
    snippet: corcell-right-slot1-encoder studio-rpc-usb-uart
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_l
    artifact-name: Corcell_L-dya-studio-xiao_ble_zmk
    snippet: corcell-left-slot1-paw3222
```

キーは `snippet:`（単数・文字列）です。`snippets:` のようにリストで書くと、
`zmkfirmware/zmk` の `build-user-config.yml` は `matrix.snippet` を空として扱い、
`west build` に `-S` が渡りません。その場合でもビルドは成功しますが、
スニペットの内容が丸ごと無視された UF2 が出力されます。

単数キーでも、文字列の中に空白で区切って複数の snippet 名を指定できます。
Zephyr が各名前に分解するため、FPC モジュールと Studio USB は併用できます。

新しい FPC モジュールを増やす場合は、`snippets/` に右手用と左手用のスニペットを追加します。
`build.yaml` には実際に取り付けたモジュールのスニペットだけを書くため、モジュール候補が増えても UF2 の出力数は増えません。

## DYA Studio の対応内容（開発者向け）

通常版との違いは DYA Studio 関連のみで、キー配線・センサー設定・ポインタの
効き方は `main` と揃えてあります。

- ZMK Studio を有効にし、ロックは無効（`CONFIG_ZMK_STUDIO_LOCKING=n`）にしています。
- BLE のブラウザ検出用に `CONFIG_ZMK_STUDIO_LOCK_BLE_DIRECT_ADVERTISING_ON_UNLOCK=y` を
  明示しています。Bluetooth で Studio を開くときは、レイヤー 3 ＋右親指の A 位置の
  `studio_unlock` を押してから機器を選択してください。
- DYA Studio が最初に読む device info は、unlock 前でも取得できるように
  `CONFIG_ZMK_DEVICE_INFO_STUDIO_RPC_REQUIRE_UNLOCK=n` を明示しています。
- Studio へは右手側の USB または Bluetooth で接続します。右手側では FPC 用と
  `studio-rpc-usb-uart` の両 snippet を、`snippet:` の空白区切り文字列で指定します。
- 2026-09-05 の `c2c3cb8` までの DYA 版は Studio の USB 接続に未対応です。
  USB でのキー入力と Studio 接続は別の機能です。更新方法は接続ガイドを参照してください。
- ポインタの倍率は `mouse_runtime_input_processor` / `scroll_runtime_input_processor`
  に持たせているので、DYA Studio から実機で調整できます。

## 電源設定

- ZMK sleep を有効にしています。
- BLE TX power は +8 dBm です。
- BLE preferred connection interval は `12-12`（15 ms）、latency は `0` です。
  これは要求値であり、ホストとの実際の接続条件は接続先にも依存します。
- 30 秒で idle、15 分で deep sleep に入る設定です。保存済みの DYA 設定がある場合はそちらも確認してください。
- PAW3222 の `force-awake` は有効にしていません。
- smooth scrolling は無効にしています。
- logging、shell、SPI shell は無効にしています。
- insomnia behavior module は含めていません。
- DYA Studio 用の runtime input processor を含めています。通常版 `main` には含めていないため、DYA Studio で保存したポインタ設定は通常版に影響しません。
- 電池残量は乾電池向け voltage divider 構成で Bluetooth の battery level として報告します。
- 分圧抵抗は `output-ohms = 470k`、`full-ohms = 1M + 470k` です。
- 1 セル Ni-MH 向けの millivolt-to-percent thresholds で Bluetooth の battery level として報告します。

## 電源投入 LED

電池を入れると、XIAO の緑 LED が 2 秒だけ点灯して消えます。
組み立て時に、ペアリングしなくても電池と昇圧回路が生きているか確認できます。

- 点灯後は GPIO を切り離すので、消えたあとの消費電流はありません。
- 点灯時間は `CONFIG_CORCELL_POWER_ON_LED_MS`（既定 2000、100〜10000 ms）で変えられます。
- 不要なら `CONFIG_CORCELL_POWER_ON_LED=n` を conf に書けば丸ごと無効化できます。
- 左右どちらの半身でも点灯します。USB 給電でも同じく点灯します。

## 更新履歴

書き込みが必要な側を「対象」に書いています。記載がない項目は左右とも書き換えてください。

### 2026-09-07 — 接続改善（実機確認待ち）

- 右手側の Studio USB 通信を有効にし、FPC センサー設定と併用しました。（対象: 右手）
- ロック無効時も `studio_unlock` で BLE のブラウザ検出用広告を開始できるようにしました。（対象: 右手）
- 初期キーマップのレイヤー 3 に、U 位置の USB 優先と I 位置の Bluetooth 優先を追加しました。
  保存済みキーマップがある場合は自動で追加されません。接続ガイドの移行手順を参照してください。（対象: 右手）
- 初回接続、旧版と新版の違い、USB の切り分け、再ペアリング、更新と初期化の手順を追加しました。
- [点検結果と実機確認項目](docs/firmware-audit-2026-09-06.md) をまとめました。

### 2026-09-05

- 無線接続時にカーソルがカクつく問題を修正しました。BLE の送信出力を 0 dBm から
  +8 dBm へ引き上げ、接続間隔を 15 ms 固定にしています。（対象: 左右）
- スリープから復帰したあとトラックボールが反応しなくなる問題を修正しました。
  復帰時にセンサーの電源管理が復旧しないため、電池を抜くまで復帰しませんでした。
  ドライバ側で対処しています。（対象: 右手）
- スリープ後にキーを押しても復帰しない問題を修正しました。右手側のキー読み取りに
  割り込み設定が抜けており、スリープ中にキー入力を検知できませんでした。（対象: 右手）

### 2026-09-03

- セッティングガイドを追加しました。ボトムケースの取り外し、マグネットの取り付け、
  チルトスタンドのサポート材除去を、動画つきで別ページにまとめています。

### 2026-08-31

- 電池を入れると XIAO の緑 LED が 2 秒点灯するようにしました。組み立て時に、
  ペアリングせずに電池と昇圧回路の動作を確認できます。（対象: 左右）
- トラックボールがまったく動作しない問題を修正しました。`build.yaml` の記述が
  ビルド側の想定と食い違っており、センサーの設定が丸ごと無視された UF2 が
  出力されていました。（対象: 左右）
- カーソル速度を調整し、基板上のロータリーエンコーダーを既定で有効にしました。
- 通常版から DYA Studio 用の設定を分離しました。DYA Studio 対応版は
  `dya-studio` ブランチで管理します。

## ライセンス

このリポジトリ内のファームウェアソースコード、ZMK 設定ファイル、ドキュメントは MIT License です。
詳しくは `LICENSE` を確認してください。

## レイヤーキーの押下確認

初期配列のL1・L2・L3キーは、押している間のレイヤー切替に加えて、それぞれF16・F17・F18を送信します。離すとキー信号とレイヤーを解除します。通常のキー検査へ押下・解放を伝えるための設定です。実機でのブラウザ検出は書き込み後に確認してください。

文字入力やIME切替用の信号は送信しません。F16〜F18にOSやアプリのショートカットが割り当てられている場合は、その動作が実行されます。短押しでI/Aになるレイヤー5キーは従来どおりです。

これは**ファームの初期配列**の変更です。Studioで保存した配列がある個体は、ファームを書き換えるだけでは新しい配列にならない場合があります。既存配列をバックアップしてから、DYA Studioの配列を初期状態へ戻す機能で適用してください（保存したキー割り当ては失われます）。全設定を消す`settings_reset`は通常不要です。

検証メモ（2026-09-18）: macOSのChromeで、OS経由のF16・F17・F18の押下・解放がKeyCheckに記録され、音声再生が始まることを確認しました。Corcell実機からのHID送信は別途確認が必要です。以前のF22〜F24はChromiumのmacOSキー対応表で未割り当てだったため取りやめました。
