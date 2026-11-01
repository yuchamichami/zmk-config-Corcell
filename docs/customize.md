# カスタマイズ

[README](../README.md) に戻る

## キーマップを変える

1. このリポジトリを fork します。
2. `config/corcell.keymap` を編集して push します。
3. Actions が自動でビルドします。完了したら Artifacts から uf2 を取得します。

ブラウザ上で編集したい場合は、DYA Studio に対応した `dya-studio` ブランチを
使ってください。ソースの編集もビルドも不要です。

出荷時のキーマップ図は [keymap-drawer/corcell.svg](../keymap-drawer/corcell.svg) にあります。

## FPC モジュールを切り替える

スロットに挿すモジュールは、Zephyr/ZMK のスニペットで切り替えます。

| モジュール | 右手 | 左手 |
|---|---|---|
| PAW3222 トラックボール | `corcell-right-slot1-paw3222` | `corcell-left-slot1-paw3222` |
| エンコーダー | `corcell-right-slot1-encoder` | `corcell-left-slot1-encoder` |

`build.yaml` に、実際に取り付けたモジュールのスニペットを 1 つだけ指定します。

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

キーは `snippet:` です。単数形・文字列で書いてください。
`snippets:` とリストで書くとビルドは成功しますが、スニペットの内容が
無視された uf2 が出力されます。

新しいモジュールを増やす場合は `snippets/` に右手用と左手用を追加します。
`build.yaml` には実際に使うものだけを書くので、候補が増えても uf2 の数は増えません。

## ポインタの速度と向き

`snippets/corcell-*-slot1-paw3222/paw3222.overlay` の input processor を変えます。

- 速度: `zip_xy_scaler 2 5` の 2 と 5。分子を上げると速くなります。
- 向き: `zip_xy_transform` に `INPUT_TRANSFORM_X_INVERT` / `Y_INVERT` /
  `XY_SWAP` を組み合わせます。
- スクロール速度: `zip_scroll_scaler 1 10`。

## 電源投入 LED

`boards/shields/corcell/corcell_*.conf` で調整します。

- `CONFIG_CORCELL_POWER_ON_LED_MS` — 点灯時間。既定 2000、範囲 100〜10000 ms
- `CONFIG_CORCELL_POWER_ON_LED=n` — 無効化

## ファームの配布

自動公開と手動での再配布は[ファームの配布](firmware-releases.md)を参照してください。
