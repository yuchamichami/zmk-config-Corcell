# Corell ZMK ファームウェア

Corell は、Corchibi のキー配置、matrix transform、physical layout、
charlieplex kscan を引き継ぎつつ、センサーを PAW3222 に変更した乾電池版の
ZMK ファームウェアです。

## ハードウェア構成

- `Corell_L` は split peripheral です。左側の PAW3222 入力を右側へ転送します。
- `Corell_R` は split central です。右側の PAW3222 と、左側から転送された入力を扱います。
- キー配線と kscan ピンは Corchibi と同じです。
- PAW3222 は `SCLK=P0.10`、`SDIO=P0.09`、`MOTION=P1.12` を使います。
- PAW3222 の NCS はデフォルトで GND 固定です。そのため、ファームウェア側では SPI chip-select GPIO を設定していません。
- NCS をジャンパで `RE_B` に接続する場合は、`&spi0` 配下に `cs-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;` を追加し、エンコーダーの B ピンを無効化するか別ピンへ移動してください。
- ロータリーエンコーダーは `RE_A=P0.04`、`RE_B=P0.05` を使います。
- 乾電池の入力電圧は ADC0 / `P0.02` で読みます。

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

## ライセンス

このリポジトリ内のファームウェアソースコード、ZMK 設定ファイル、ドキュメントは MIT License です。
詳しくは `LICENSE` を確認してください。
