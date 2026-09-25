# lenoTP 新FPCの初回テスト（2026-09-25）

対象は、従来の `lenotp` 実験ブランチで使ったI²Cトラックポイントを、新FPCでPAW3222の代わりに接続する構成です。ドライバーは `yuchamichami/zmk-driver-lenotp` の `3e5e9926d243ed09d62a833c5664278e9fe5a41b` を使用します。センサー本体・プロトコルは従来と同じ想定で、新基板の実機動作は未確認です。

## 接続

**電源OFF・USBを抜いた状態で、PAWを外して新FPCに差し替えます。** 表はFPCコネクタの端子番号で、MCUのGPIO番号ではありません。端子番号はコネクタの番号・向きに合わせて確認してください。

| FPC端子 | 元のPAW信号 | 新FPC信号 | CorcellのGPIO |
| --- | --- | --- | --- |
| 1 | NCS | NC（モジュール側未接続） | 割り当てなし |
| 2 | SCLK | SCL | P0.10 |
| 3 | MOTION | INT | P1.12、Lowで割り込み |
| 4 | SDIO | SDA | P0.09 |
| 5 | 3V3 | 3.3V電源 | 3V3 |
| 6 | GND | GND | GND |

SCL・SDAは各4.7kΩ、INTは10kΩで3.3Vへ外付けプルアップされています。これに合わせてスロット用の内部プルアップを無効にしています。

I²Cは100kHz・7bitアドレス0x15、`i2c0` を使用します。同じ周辺回路を共有する `spi0` とPAWドライバーは無効です。P0.09/P0.10はNFC機能を無効にしてGPIOとして使用します。1番ピンをリセットや電源制御には使いません。

## まず右側USBで確認

1. 右側のPAWを新FPCのトラックポイントへ差し替えます。
2. 右側のリセットを素早く2回押してUF2ドライブを出し、`Corcell_R-lenotp-FPC-diagnostics-xiao_ble_zmk.uf2` を1ファイルだけコピーします。
3. 通常起動後、右側をUSBでPCへ接続し、トラックポイントを倒してカーソルが動くか確認します。DYA Studio接続は不要です。
4. 向き・速度・無操作時のドリフトを確認します。この版では軸反転や倍率を新たに決め打ちしていません。
5. 問題があれば診断版のままログを取ります。通信が正常に通ってから速度や向きを調整します。

この版は従来のlenotpキーマップを使用します。DYA Studioで保存した配列・ポインター調整の再現を保証するものではありません。右側だけのカーソル確認に左側の書換えやペアリング消去は不要です。

## ファイルの選び方

| ファイル | 用途 |
| --- | --- |
| `Corcell_R-lenotp-FPC-diagnostics-xiao_ble_zmk.uf2` | **最初はこちら。右側PAWコネクタ用・USB診断ログあり** |
| `Corcell_L-lenotp-FPC-diagnostics-xiao_ble_zmk.uf2` | 左側PAWコネクタ用・USB診断ログあり。カーソルは右側経由でPCへ送信 |
| `Corcell_R-lenotp-xiao_ble_zmk.uf2` | 右側PAWコネクタ用・通常版 |
| `Corcell_L-lenotp-xiao_ble_zmk.uf2` | 左側PAWコネクタ用・通常版 |
| `Corcell_R-lenotp-BENCH-xiao_ble_zmk.uf2` | 旧来のXIAO D4/D5直結用。**今回のFPCには使わない** |
| `settings_reset-lenotp-xiao_ble_zmk.uf2` | ペアリング等の保存設定の消去用。初回テストでは不要 |

診断版もカーソルを出力します。USBログ・シェルを有効にし、確認中の自動ディープスリープは無効にしています。通常版は従来のスリープ設定を維持します。電池寿命の評価には通常版を使います。

## ログの見方

USBシリアルを開いたまま右側のリセットを1回押すと、起動ログを取得できます。Studioなど同じポートを使うアプリは閉じます。

- `lenoTP input initialized`: 起動時Wake upコマンドと割り込みの設定が成功。
- `buttons=.. x=.. y=..`: 受信データ。スティックを倒してX/Yが変わるか確認。
- `Failed to run init command`: 起動時のI²Cコマンドが失敗。配線・FFC接点・電源・アドレスを確認。
- `Failed to read report` / `Invalid report header`: 通信エラーまたは想定と異なるレポート。

診断版にはI²C/GPIOシェルも含めます。追加の切り分けが必要な場合に使用します。旧BENCHのバス `i2c@40004000` と、今回のFPCの `i2c@40003000` を混同しないでください。まずはドライバーのログを使い、初回から手動スキャンを行う必要はありません。

## 再ビルド

このリポジトリの `config/west.yml` をmanifestにしたwest workspaceで、Corcellリポジトリの絶対パスを `CORCELL_CONFIG_ROOT` に設定します。

```sh
west update
west zephyr-export
west build -p always -d build-lenotp-right -s zmk/app \
  -b xiao_ble/nrf52840/zmk \
  -S 'corcell-right-slot1-lenotp corcell-lenotp-diagnostics' -- \
  -DSHIELD=corcell_r \
  -DZMK_CONFIG="$CORCELL_CONFIG_ROOT/config" \
  -DZMK_EXTRA_MODULES="$CORCELL_CONFIG_ROOT"
```

生成物は `build-lenotp-right/zephyr/zmk.uf2`。通常版は診断snippetを除き、左側は `corcell_l` と `corcell-left-slot1-lenotp` に置き換えます。GitHub Actionsは `build.yaml` の各構成をビルドします。

ビルド成功は実機でのI²C応答・移動・スリープ復帰の確認とは区別します。
