# Corcell — 通常版

Corcellは、右手にトラックボールを備えた左右分割キーボードです。左右それぞれ単四電池1本で動きます。

この`main`ブランチは、DYA Studioを使わない通常版です。出荷用のDYA Studio版は[dya-studioブランチ](https://github.com/yuchamichami/zmk-config-Corcell/tree/dya-studio)にあります。
ブラウザでキー配列やボールの感度を変えたい場合は、そちらを使ってください。

## ファームのダウンロード

<!-- firmware-downloads:start -->
<!-- firmware-build: 41304609549 1 -->
[左右まとめてダウンロード（ZIP）](https://github.com/yuchamichami/zmk-config-Corcell/releases/download/firmware-main-41304609549-1/Corcell-main-LR.zip)

| 個別ダウンロード | 用途 |
| --- | --- |
| [左手用（L）](https://github.com/yuchamichami/zmk-config-Corcell/releases/download/firmware-main-41304609549-1/Corcell_L-xiao_ble_zmk.uf2) | 左に書き込むUF2 |
| [右手用（R）](https://github.com/yuchamichami/zmk-config-Corcell/releases/download/firmware-main-41304609549-1/Corcell_R-xiao_ble_zmk.uf2) | 右に書き込むUF2 |
| [設定リセット用](https://github.com/yuchamichami/zmk-config-Corcell/releases/download/firmware-main-41304609549-1/settings_reset-xiao_ble_zmk.uf2) | ペアリング・保存した設定をすべて消す場合のみ |

配布元：2026-11-01 / `d1866c6` · [配布ファイルの詳細](https://github.com/yuchamichami/zmk-config-Corcell/releases/tag/firmware-main-41304609549-1)
<!-- firmware-downloads:end -->

GitHubへのログインは不要です。ZIPには左右のUF2が入っています。通常の更新では設定リセット用は使いません。

## 接続する

PCやスマホにつながるのは右手側です。左手の入力は右手へBluetoothで送るため、USBで使うときも左手には電池を入れて電源を入れてください。

### USBで使う

データ通信できるUSBケーブルで右手側とPCをつなぎ、メモ帳などで左右の文字入力と右のボールを確認します。
通常版の初期配列には、DYA版の「レイヤー3＋U／I」による出力切替はありません。

### Bluetoothで使う

1. USBを外し、左右の電源を入れます。
2. レイヤー3キーを押しながら右上端のBackspaceを押し、接続先0を選びます。
3. PCやスマホのBluetooth設定を開き、Corcellを追加します。左右を別々に登録する必要はありません。

レイヤー3キーは、右親指のSpaceと、短押しでAになるキーの間です。[初期配列の図](keymap-drawer/corcell.svg)も確認できます。

接続先0を別の端末で使っている場合は、空いている番号を選んでください。どれもレイヤー3キーを押しながら操作します。

| キーの位置 | 接続先 |
| --- | --- |
| 右上端のBackspace | 0 |
| 右端のEnter | 1 |
| 右端の `?` | 2 |
| 右下端の → | 3 |

## ファームを更新する

1. [上のダウンロード](#ファームのダウンロード)から左右のZIPを保存し、展開します。
2. 書き込む側をUSBでつなぎ、XIAOのリセットボタンを素早く2回押します。
3. 表示されたUSBドライブへ、その側のUF2を1個だけコピーします。書き込みが終わるとドライブが消えて再起動します。
4. もう片側も同じ手順で書き込みます。

同じZIPの左右をそろえてください。通常の更新ではペアリングを引き継ぎます。
Windowsでコピー中にエラーが出た場合は、[書き込みエラーの確認方法](https://github.com/yuchamichami/zmk-config-Corcell/blob/dya-studio/docs/connection-guide.md#windows-の書き込みで0x80070022と出る)を参照してください。

### スリープ不具合の修正版へ更新する

2026年11月1日に、左手がスリープから起きない不具合と、左右共通のスリープ処理を修正しました。[上のダウンロード](#ファームのダウンロード)から、左右両方を更新してください。

更新後は左右のUSBを外し、電池で15分以上操作せずに待ちます。左右それぞれのキーを押してLEDが点き、数秒後に文字入力と右のボールが戻るか確認してください。最初の1打は復帰に使われ、文字として出ないことがあります。

左右のビルドと設定の検査は済んでいます。この修正版での実機確認は進行中です。[変更の詳細](docs/changelog.md)も記載しています。

## ケースを開ける

V2は、ボトムケースとフレームを合わせたままトッププレートを外します。
R側は先にトラックボールケースの固定ネジを上から外してください。[詳しい分解手順](docs/v2-case-disassembly.md)はこちらです。

https://github.com/user-attachments/assets/4051cda4-3a6b-4ba5-9c34-13392f8c34de

マグネットやチルトスタンドについては、[セッティングガイド](docs/setup-guide.md)を参照してください。

## 困ったとき・設定を変えたいとき

- [Discordサーバー](https://discord.gg/nryEcZ6YjU)：接続方法と症状、使ったファームのBuild URLを添えて相談してください。
- [カスタマイズ](docs/customize.md)：ソースでのキーマップ変更、FPCモジュールの切替、ボールの調整。
- [ハードウェア構成](docs/hardware.md)：ピン配置、電源、無線の設定。
- [更新履歴](docs/changelog.md)：これまでの変更。

`lenotp`はトラックポイントの実験用ブランチです。

## ライセンス

ファームのソース、ZMK設定、ドキュメントは[MIT License](LICENSE)です。
