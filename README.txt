シミュレータだよ


使用方法：
1: make
2: ./simulator "実行ファイル" [-p|--printinfo] [-c|--countop] [-n|--nativeFPU] [-d|--debug]

・-p / --printinfo
終了時にレジスタの値と命令実行数を吐く

・-c / --countop
終了時に命令実行数と書く命令毎の実行数を吐く

・-n / --nativeFPU
FPUをcの方で実行する。
元の方のも実行して保持してあるがmemory命令はまだなので意味は薄い。

・debug mode
引数に -d か --debug をつけるとデバッグモード
コマンドはヘルプ参照



更新履歴：
11/8/2:00/tmp
オプションの引数を変更。
FPUをネイティブにする機能を実装。

11/5/03:00/op count
実行命令をカウントする機能を実装。
  実行時引数に -p をつけるべし

11/4/14:00/debugger03
デバッガ機能追加
  break作った
  デバッガ仕様書作った

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
