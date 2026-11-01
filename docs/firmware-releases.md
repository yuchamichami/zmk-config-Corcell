# ファームの配布

[README](../README.md)のリンクから、GitHubへのログインなしでUF2をダウンロードできます。
左右用ZIPにはL/Rだけを入れ、設定リセット用は個別に配布します。

## 自動公開

`main`・`dya-studio`のFirmwareビルド成功後、`Publish firmware downloads`が配布します。
PR・実験ブランチ・Draw Keymapの実行結果は自動公開しません。README・ドキュメント・配列図だけの変更ではファームをビルドしません。

配布処理では、ビルド元のソースが対象ブランチと一致すること、3つのUF2のファイル名・形式・書込み範囲を確認します。
ファイルをReleasesへアップロードし、再ダウンロードした内容が一致してからREADMEのリンクを更新します。
ZIPと個別ファイルのリンクは、同じビルドを指します。以前の配布物は上書きしません。

公開中にブランチが変更された場合はREADME更新を停止します。新しいビルドが必要ならその完了を待ち、文書の変更だけなら配布処理を再実行してください。
自動公開で確認するのはビルドと配布物です。実機での入力・ボール・スリープ復帰は別途確認します。

## 成功済みのビルドを手動で配布する

1. Actionsの`Publish firmware downloads`を開き、`Run workflow`を選びます。
2. ワークフローを実行するブランチは`main`を選びます。
3. `run_id`にFirmwareビルドのURL末尾の数字、`branch`に配布先の`main`または`dya-studio`を指定します。

対象ブランチに取り込まれたコミットで、ファームのソースが現在と同じ場合に限り配布できます。
既に新しいビルドへ更新済みなら、READMEを古い配布物へ戻しません。

ローカルで公開前の検査だけを行う場合：

```sh
python3 scripts/publish_firmware.py --run-id BUILD_ID --branch dya-studio --work-dir /path/to/work --prepare-only
```

配布処理のテストは`python3 -m unittest discover -s tests`で実行できます。
