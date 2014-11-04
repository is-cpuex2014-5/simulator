シミュレータだよ


使用方法：
1: make
2: ./simulator [実行ファイル]

・debug mode
引数の最後に -d か --debug をつけるとデバッグモード
コマンドはヘルプ参照

・エンディアン変更に伴い既存のtestsは動かない


更新履歴：
11/4/11:00/debuger02
デバッガ機能追加
  printにoption([i,f, ]rg, op)追加
  listを追加

10/29/17:00/debuger01
デバッグ機能追加（デバッグ中）
step,print,continueくらいは動くっぽい。

10/28/14:00/bugfix: b(f)lt
branchまわりのバグ修正（ごめんなさい）

10/28/01:00/readwrite
read/write仮実装
諸々増えていたgomi(branch関連：gitの)を消去

10/27/18:00/moromoro
fpu導入
関数をheaderに分けた

10/27/14:00/some fixes
store/loadを実装
shiftを実装
Makefileを変更

10/24/14:10/endian fix
エンディアン問題を解決

10/22/02:40/~branch
命令読み込み方式の変更
ALUとBranchの動作確認
loop版fibの動作確認
