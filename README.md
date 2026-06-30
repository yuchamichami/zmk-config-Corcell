# Corchibi2 ZMK ファームウェア

Corchibi2 は、Corchibi のキー配置、matrix transform、physical layout、
charlieplex kscan を引き継ぎつつ、センサーを PAW3222 に変更した乾電池版の
ZMK ファームウェアです。

## ハードウェア構成

- `Corchibi2_L` は split peripheral です。左側の PAW3222 入力を右側へ転送します。
- `Corchibi2_R` は split central です。右側の PAW3222 と、左側から転送された入力を扱います。
- キー配線と kscan ピンは Corchibi と同じです。
- PAW3222 は `SCLK=P0.10`、`SDIO=P0.09`、`MOTION=P1.12` を使います。
- PAW3222 の NCS はデフォルトで GND 固定です。そのため、ファームウェア側では SPI chip-select GPIO を設定していません。
- NCS をジャンパで `RE_B` に接続する場合は、`&spi0` 配下に `cs-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;` を追加し、エンコーダーの B ピンを無効化するか別ピンへ移動してください。
- ロータリーエンコーダーは `RE_A=P0.04`、`RE_B=P0.05` を使います。
- 乾電池の入力電圧は ADC0 / `P0.02` で読みます。

## IQS5xx トラックパッド実験ブランチ

`experiment/trackpad-iqs5xx` ブランチでは、PAW3222 の代わりに Azoteq IQS5xx 系
トラックパッドを使う構成を試しています。

- `SDA=P0.09`、`SCL=P0.10`、`RDY=P1.12` です。
- `RST` は PAW3222 の NCS と同じコネクタ位置ですが、未接続のためファームウェアでは使いません。
- I2C アドレスは `0x74` です。
- 通常のビルド対象は `Corchibi2_R`、`Corchibi2_L`、`settings_reset` です。

### トラックパッドのデバッグ

トラックパッドが反応しない場合は、`Corchibi2_R_trackpad_debug` を右手側へ書き込むと
USBシリアルログで状態を確認できます。

1. `Corchibi2_R_trackpad_debug-xiao_ble.uf2` を右手側に書き込みます。
2. 右手側をUSB接続します。
3. macOS でシリアルポートを確認します。

```sh
ls /dev/tty.usbmodem*
```

4. 見つかったポートを開きます。

```sh
screen /dev/tty.usbmodemXXXX 115200
```

ログでは次の行を確認します。

- `IQS5xx debug probe start`: デバッグプローブが起動しています。
- `RDY sample[...]`: `RDY=P1.12` の入力状態です。
- `probe addr=0x74 ok`: I2C アドレス `0x74` でトラックパッドが応答しています。
- `probe addr=0x74 no response`: 電源、SDA/SCL、I2C アドレス、RST 周辺を疑います。
- `trackpad_event ...`: トラックパッド入力がZMKの入力処理まで届いています。

## 電源設定

- ZMK sleep を有効にしています。
- BLE TX power は Corchibi の +8 dBm ではなく、0 dBm にしています。
- BLE preferred connection interval は `6-12`、latency は `0` にして、ポインタ操作の遅延を抑えています。
- PAW3222 の `force-awake` は有効にしていません。
- ZMK Studio は無効にしています。
- smooth scrolling は無効にしています。
- logging、shell、SPI shell は無効にしています。
- insomnia behavior module は含めていません。
- 電池残量は DYA Dash の乾電池向け voltage divider 構成を参考にしています。
- 分圧抵抗は `output-ohms = 470k`、`full-ohms = 1M + 470k` です。
- 1 セル Ni-MH 向けの millivolt-to-percent thresholds で Bluetooth の battery level として報告します。

## 3D データ

ケースなどの 3D モデルは `3D_data/` にあります。

3D モデルはファームウェアとは別ライセンスです。MIT License の対象ではありません。
商用利用はできません。改変は私的利用に限ります。改変した 3D モデルや派生物の共有、配布、販売はできません。

詳しくは `3D_data/README.md` と `3D_data/LICENSE.md` を確認してください。

## ライセンス

このリポジトリは、ファームウェアと 3D モデルでライセンスが分かれています。

- ファームウェアのソースコード、ZMK 設定ファイル、`3D_data/` 外のドキュメントは MIT License です。詳しくは `LICENSE` を確認してください。
- `3D_data/` 以下の 3D モデルファイルは CC BY-NC-ND 4.0 です。商用利用はできません。改変は私的利用に限ります。改変したデータや派生物の共有、配布、販売はできません。詳しくは `3D_data/LICENSE.md` を確認してください。
