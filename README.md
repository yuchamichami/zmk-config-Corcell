# Corcell ZMK ファームウェア

Corcell は、PAW3222 トラックボールと乾電池駆動に対応した ZMK ファームウェアです。
キー配線、matrix transform、physical layout、charlieplex kscan は Corchibi 互換です。

**このブランチ（`lenotp`）は、FPC スロット 1 のセンサーを PAW3222 から
lenoTP に置き換えた版です。**
通常版は `main`、DYA Studio 対応版は `dya-studio` ブランチです。

lenoTP モジュールは PAW3222 とピン配置互換なので、スロット 1 のネットを
そのまま流用します。

| FPC | lenoTP | PAW3222 での役割 | XIAO |
|---|---|---|---|
| 1 | NC | NCS | P0.05（RE_B）・未使用 |
| 2 | SDA | SDIO | `P0.09` |
| 3 | SCL | SCLK | `P0.10` |
| 4 | INT | MOTION | `P1.12` |
| 5 | VCC | VCC | 3V3 |
| 6 | GND | GND | GND |

- ドライバは [`zmk-driver-lenotp`](https://github.com/yuchamichami/zmk-driver-lenotp) です。
  I2C アドレスは `0x15`、`INT` は `GPIO_ACTIVE_LOW | GPIO_PULL_UP` で受けます。
- nRF52840 の `i2c0` と `spi0` は同一インスタンス（どちらも `0x40003000`）なので、
  snippet 側で `spi0` を無効化しています。PAW3222 との併用はできません。
- `P0.09` / `P0.10` は NFC ピンですが、`corcell.dtsi` の `nfct-pins-as-gpios` で
  GPIO として使える状態にしてあります。
- 電池駆動のため `sleep-on-suspend` / `wakeup-on-resume` を有効にしています。
- **向きと速度は未調整です。** 実機で確認したうえで、ドライバの
  `invert-x` / `invert-y` / `swap-xy` / `x-divisor` / `y-divisor` で合わせてください。

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
通常の `build.yaml` では PAW3222 snippet だけを指定しているため、生成される UF2 の数は増えません。

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
