# Twist debug experiment

PAW3222 の生の相対移動量を USB serial log に出す実験用ファームです。
通常の `Corell_R` とは別に、`Corell_R_twist_debug` shield として用意しています。

## 目的

玉を左右に捻ったときに、PAW3222 から出る `x` / `y` の値へ通常の上下移動とは違う癖があるか確認します。
このファームではカーソル処理の前に `INPUT_REL_X` / `INPUT_REL_Y` を記録し、以下のようなログを出します。

```text
twist_raw seq=12 dt_ms=15 x=-2 y=18 abs_x=2 abs_y=18 dom=y
```

## ビルド

```sh
west build -s zmk/app -d .build/Corell_R_twist_debug-xiao_ble \
  -b xiao_ble --pristine always -- \
  -DZMK_CONFIG="$PWD/config/zmk-config-Corell/config" \
  -DZMK_EXTRA_MODULES="$PWD/config/zmk-config-Corell" \
  -DSHIELD=Corell_R_twist_debug \
  -DSNIPPET=zmk-usb-logging
```

## ログを見る

デバッグファームを書き込んだあと、USB 接続した状態で以下を実行します。

```sh
ls /dev/tty.usbmodem* /dev/cu.usbmodem* 2>/dev/null
screen /dev/tty.usbmodemXXXX 115200
```

終了は `Ctrl-a` のあと `k`、続けて `y` です。

ログをファイルに残す場合は、リポジトリ直下から以下を実行します。

```sh
./experiments/twist_debug/capture_twist_log.sh
```

## 観測メモ

同じくらいの強さで次を数回ずつ行い、`x` / `y` / `dom` の傾向を比べます。

- 通常の上移動
- 通常の下移動
- 右捻り
- 左捻り

右捻り・左捻りだけ `x` と `y` の比率、符号、連続時間、ピーク値に偏りがあれば、その条件を元にスクロール変換 processor を作れます。
