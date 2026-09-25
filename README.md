# Corcell ZMK ファームウェア

Corcell は、PAW3222 トラックボールと乾電池駆動に対応した ZMK ファームウェアです。
キー配線、matrix transform、physical layout、charlieplex kscan は Corchibi 互換です。

**このブランチ（`lenotp`）は、FPC スロット 1 のセンサーを PAW3222 から
lenoTP に置き換えた実験中の版です。一般利用には `main` または `dya-studio` を選んでください。**
通常版は `main`、DYA Studio 対応版は `dya-studio` ブランチです。

今回の新しい変換FPCを介して、PAW3222用スロット1へlenoTPを接続します。
以下はコネクタ端子番号です。旧READMEの2〜4番の表記を修正しました。GPIOの割り当て自体は従来と同じです。

| FPC | lenoTP | PAW3222 での役割 | XIAO |
|---|---|---|---|
| 1 | NC（モジュール側未接続） | NCS | 使用しない |
| 2 | SCL | SCLK | `P0.10` |
| 3 | INT | MOTION | `P1.12` |
| 4 | SDA | SDIO | `P0.09` |
| 5 | VCC | VCC | 3V3 |
| 6 | GND | GND | GND |

- ドライバは [`zmk-driver-lenotp`](https://github.com/yuchamichami/zmk-driver-lenotp) です。
  I2C アドレスは `0x15`、`INT` は `GPIO_ACTIVE_LOW` で受けます。
- 新FPC上の外付けプルアップ（SCL/SDA各4.7kΩ、INT 10kΩ、すべて3.3V）を使い、
  スロット1の内部プルアップは無効にしています。I2Cは100kHzです。
- **初回テストは [新FPCの接続・書込み手順](docs/lenotp-fpc-test.md) を参照してください。**
  `FPC-diagnostics` はPAWコネクタ用、旧 `BENCH` はXIAO D4/D5直結用で、配線が異なります。
- nRF52840 の `i2c0` と `spi0` は同一インスタンス（どちらも `0x40003000`）なので、
  snippet 側で `spi0` を無効化しています。PAW3222 との併用はできません。
  未使用の `i2c1` も無効にし、エンコーダーのP0.04/P0.05を占有しないようにしています。
- `P0.09` / `P0.10` は NFC ピンですが、`corcell.dtsi` の `nfct-pins-as-gpios` で
  GPIO として使える状態にしてあります。
- 電池駆動のため `sleep-on-suspend` / `wakeup-on-resume` を有効にしています。
- **向きと速度は未調整です。** 実機で確認したうえで、ドライバの
  `invert-x` / `invert-y` / `swap-xy` / `x-divisor` / `y-divisor` で合わせてください。

## ブランチの選び方

利用者向けは **`main` と `dya-studio`** です。各ブランチの README にセットアップ手順を記載しています。

| ブランチ | 用途 |
| --- | --- |
| [`main`](https://github.com/yuchamichami/zmk-config-Corcell/tree/main) | 通常版。ブラウザでの設定編集を使わない方向け |
| [`dya-studio`](https://github.com/yuchamichami/zmk-config-Corcell/tree/dya-studio) | DYA Studio 対応版。ブラウザでキーマップやポインタ倍率を調整する方向け |
| [`lenotp`](https://github.com/yuchamichami/zmk-config-Corcell/tree/lenotp) | lenoTP の実験用。通常の利用者向け配布版ではありません |

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
- PAW3222 の CPI はファームウェア側で上書きせず、カーソル移動量は固定の input processor 倍率
  （`zip_xy_scaler 2 5`、スクロールは `zip_scroll_scaler 1 10`）で調整します。
- 基板上のロータリーエンコーダーは `RE_A=P0.04`、`RE_B=P0.05` で、デフォルトで有効です。
- FPC エンコーダーモジュールでは、PAW3222 の `NCS` 位置を A 相、`MOTION` 位置を B 相として使います。
- 現行回路では、エンコーダーモジュール使用時に `NCS` を `RE_B` 側へジャンパしてください。このときファームウェアは `A=P0.05`、`B=P1.12` として読みます。
- 乾電池の入力電圧は ADC0 / `P0.02` で読みます。

## FPC モジュールの切り替え

FPC モジュールは Zephyr/ZMK のスニペットで切り替えます。
この `lenotp` ブランチの `build.yaml` は左右のlenoTP通常版・FPC診断版と、旧BENCH版・settings_resetを生成します。

- 右手 PAW3222: `corcell-right-slot1-paw3222`
- 左手 PAW3222: `corcell-left-slot1-paw3222`
- 右手エンコーダー: `corcell-right-slot1-encoder`
- 左手エンコーダー: `corcell-left-slot1-encoder`

たとえば右手スロットをエンコーダーにする場合は、`build.yaml` の
`corcell-right-slot1-paw3222` を `corcell-right-slot1-encoder` に変更します。

ユーザー目線では次の流れです。

1. 左右それぞれ、FPC スロットに取り付けるモジュールを決めます。
2. `build.yaml` の `corcell_r` と `corcell_l` に、取り付けたモジュールのスニペットを 1 つだけ指定します。
3. 変更を push します。
4. GitHub Actions の `Build` が完了したら、Artifacts から UF2 をダウンロードします。
5. `Corcell_R-...uf2` を右手、`Corcell_L-...uf2` を左手に書き込みます。

たとえば右手をエンコーダー、左手を PAW3222 にする場合は次のようにします。

```yaml
include:
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_r
    artifact-name: Corcell_R-xiao_ble_zmk
    snippet: corcell-right-slot1-encoder
  - board: xiao_ble/nrf52840/zmk
    shield: corcell_l
    artifact-name: Corcell_L-xiao_ble_zmk
    snippet: corcell-left-slot1-paw3222
```

キーは `snippet:`（単数・文字列）です。`snippets:` のようにリストで書くと、
`zmkfirmware/zmk` の `build-user-config.yml` は `matrix.snippet` を空として扱い、
`west build` に `-S` が渡りません。その場合でもビルドは成功しますが、
スニペットの内容が丸ごと無視された UF2 が出力されます。

新しい FPC モジュールを増やす場合は、`snippets/` に右手用と左手用のスニペットを追加します。
`build.yaml` には実際に取り付けたモジュールのスニペットだけを書くため、モジュール候補が増えても UF2 の出力数は増えません。

## 電源設定

- ZMK sleep を有効にしています。
- BLE TX power は Corchibi の +8 dBm ではなく、0 dBm にしています。
- BLE preferred connection interval は `6-12`、latency は `0` にして、ポインタ操作の遅延を抑えています。
- PAW3222 の `force-awake` は有効にしていません。
- smooth scrolling は無効にしています。
- logging、shell、SPI shell は無効にしています。
- insomnia behavior module は含めていません。
- 通常版 `main` では DYA Studio 用の runtime input processor を含めていません。DYA Studio で保存したポインタ設定が通常版に影響しない構成です。
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
