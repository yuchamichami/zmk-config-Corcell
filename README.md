# Corcell — DYA Studio版

Corcellは、右手にトラックボールを備えた左右分割キーボードです。左右それぞれ単四電池1本で動きます。

この`dya-studio`ブランチは出荷用のファームです。[DYA Studio](https://studio.dya.cormoran.works/)でキー配列やボールの感度を変更できます。
Studioを使わない[通常版はmain](https://github.com/yuchamichami/zmk-config-Corcell/tree/main)にあります。

## 接続する

USBは右手側につなぎます。左手の入力は右手へBluetoothで送るため、USBで使うときも左手には電池を入れて電源を入れてください。

下の「レイヤー3＋U」は、右親指のレイヤー3キーを押しながら、Uの位置を1回押す操作です。
レイヤー3キーは、右親指のSpaceと、短押しでAになるキーの間にあります。[初期配列の図](keymap-drawer/corcell.svg)も確認できます。
配列を変更している場合は、自分で割り当てたキーを使ってください。

### USBで使う

1. データ通信できるUSBケーブルで、右手側とPCをつなぎます。
2. レイヤー3＋Uで出力先をUSBにします。
3. メモ帳などで左右の文字入力と、右のボールを確認します。

USBを挿したままBluetoothで入力したいときは、レイヤー3＋Iで切り替えます。

### Bluetoothで使う

1. USBを外し、左右の電源を入れます。
2. レイヤー3＋右上端のBackspaceを押して、接続先0を選びます。
3. PCやスマホのBluetooth設定を開き、Corcellを追加します。左右を別々に登録する必要はありません。

接続先0を別の端末で使っている場合は、[空いている接続先](docs/connection-guide.md#4-接続操作の早見表)を選んでください。
文字入力とボール操作だけなら、Studioを開く必要はありません。

## DYA Studioで配列や感度を変える

上の手順でUSB接続してから、PCのChromeかEdgeで[DYA Studio](https://studio.dya.cormoran.works/)を開きます。

1. StudioのUSB接続を選び、Corcellのポートを選びます。
2. 配列やボールの感度を編集して保存します。
3. 保存後は10秒以上待ってから電源を切ります。

USB接続ではStudio Unlockの操作は不要です。ポートが2つ表示されて一方で応答しない場合は、その接続を閉じてもう一方を試してください。

BluetoothでStudioにつなぐ場合は、先にOSとペアリングし、レイヤー3＋右親指のAを押します。
このStudio Unlock操作で、ブラウザからCorcellを見つけられるようになります。[BluetoothでのStudio接続手順](docs/connection-guide.md#bluetooth-接続)も参照してください。

古いファームや保存済みの配列には、USB／Bluetooth切替キーがない場合があります。[配列を引き継いで更新する場合](docs/connection-guide.md#保存済み配列がある場合)を確認してください。

## ファームを更新する

[GitHub Actionsのdya-studioビルド](https://github.com/yuchamichami/zmk-config-Corcell/actions/workflows/build.yml?query=branch%3Adya-studio)から、成功したBuildのArtifactsにある`firmware`をダウンロードして展開します。ダウンロードにはGitHubへのログインが必要です。

| ファイル | 書き込む側 |
| --- | --- |
| `Corcell_L-dya-studio-xiao_ble_zmk.uf2` | 左 |
| `Corcell_R-dya-studio-xiao_ble_zmk.uf2` | 右 |
| `settings_reset-dya-studio-xiao_ble_zmk.uf2` | 設定をすべて消すときだけ使用 |

書き込みは[ファーム更新の手順](docs/connection-guide.md#8-ファーム更新)に沿って、左右1台ずつ行います。通常の更新では保存済みの配列とペアリングを引き継ぐので、`settings_reset`は使いません。

### スリープ不具合の修正版へ更新する

2026年11月1日に、左手がスリープから起きない不具合と、左右共通のスリープ処理を修正しました。[修正版のビルド](https://github.com/yuchamichami/zmk-config-Corcell/actions/runs/41304639786)から、左右両方を更新してください。

更新後は左右のUSBを外し、電池で15分以上操作せずに待ちます。左右それぞれのキーを押してLEDが点き、数秒後に文字入力と右のボールが戻るか確認してください。最初の1打は復帰に使われ、文字として出ないことがあります。
Studioでスリープ時間を変更済みなら、その時間に合わせて待ってください。

左右のビルドと設定の検査は済んでいます。この修正版での実機確認は進行中です。[変更の詳細](docs/changelog.md)も記載しています。

## ケースを開ける

V2は、ボトムケースとフレームを合わせたままトッププレートを外します。
R側は先にトラックボールケースの固定ネジを上から外してください。[詳しい分解手順](docs/v2-case-disassembly.md)はこちらです。

https://github.com/user-attachments/assets/4051cda4-3a6b-4ba5-9c34-13392f8c34de

マグネットやチルトスタンドについては、[セッティングガイド](docs/setup-guide.md)を参照してください。

## 困ったとき

[接続・使い方ガイド](docs/connection-guide.md)に、Studioの接続エラー、ペアリングのやり直し、書き込み時のエラーをまとめています。
解決しない場合は、[Discordサーバー](https://discord.gg/nryEcZ6YjU)で[問い合わせ用の項目](docs/connection-guide.md#問い合わせ時に伝える内容)を添えて相談してください。

ソースを変更する場合は[ファームの設定・改造](docs/development.md)、過去の変更は[更新履歴](docs/changelog.md)を参照してください。
`lenotp`はトラックポイントの実験用ブランチです。

## ライセンス

ファームのソース、ZMK設定、ドキュメントは[MIT License](LICENSE)です。
