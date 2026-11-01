# ファームの設定・改造

[README](../README.md)に戻る

通常の接続やStudioでの設定変更は、READMEと接続ガイドを参照してください。ここではソースを変更する場合の設定を扱います。

## ハードウェア構成

- `corcell_l` は split peripheral です。左側のモジュール入力を右側へ転送します。
- `corcell_r` は split central です。右側のモジュール入力と、左側から転送された入力を扱います。
- キー配線と kscan ピンは Corchibi と同じです。
- 6 ピン FPC スロットには、左右それぞれ 1 つずつ任意の入力モジュールを接続できます。
- 出荷品のボールは右側だけです。ファームは左右のPAW3222を有効にしており、左側への追加にも対応する設定になっています。
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

変更してビルドする手順は次のとおりです。

1. 左右それぞれ、FPC スロットに取り付けるモジュールを決めます。
2. `build.yaml` の `corcell_r` と `corcell_l` の FPC 用 snippet を選びます。右手側の `studio-rpc-usb-uart` は残します。
3. 変更を push します。
4. GitHub Actions の `Firmware` が完了したら、Artifacts から UF2 をダウンロードします。
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

DYA版では、以下の設定でStudioの機能を有効にしています。

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
組み立て時の起動確認に使えます。点灯だけで無線接続やセンサーの動作までは判断できません。

- 点灯後はLEDのGPIOを切り離します。
- 点灯時間は `CONFIG_CORCELL_POWER_ON_LED_MS`（既定 2000、100〜10000 ms）で変えられます。
- 不要なら `CONFIG_CORCELL_POWER_ON_LED=n` を conf に書けば丸ごと無効化できます。
- 左右とも点灯します。USB 給電でも同じく点灯します。

## レイヤーキーの押下確認

初期配列のL1・L2・L3キーは、押している間のレイヤー切替に加えて、それぞれF16・F17・F18を送信します。離すとキー信号とレイヤーを解除します。通常のキー検査へ押下・解放を伝えるための設定です。実機でのブラウザ検出は書き込み後に確認してください。

文字入力やIME切替用の信号は送信しません。F16〜F18にOSやアプリのショートカットが割り当てられている場合は、その動作が実行されます。短押しでI/Aになるレイヤー5キーは従来どおりです。

これはファームの初期配列の変更です。Studioで保存した配列がある個体は、ファームを書き換えるだけでは新しい配列にならない場合があります。既存配列をバックアップしてから、DYA Studioの配列を初期状態へ戻す機能で適用してください（保存したキー割り当ては失われます）。全設定を消す`settings_reset`は通常不要です。

検証メモ（2026-09-18）: macOSのChromeで、OS経由のF16・F17・F18の押下・解放がKeyCheckに記録され、音声再生が始まることを確認しました。Corcell実機からのHID送信は別途確認が必要です。以前のF22〜F24はChromiumのmacOSキー対応表で未割り当てだったため取りやめました。

## ファームの配布

自動公開と手動での再配布は[ファームの配布](firmware-releases.md)を参照してください。
