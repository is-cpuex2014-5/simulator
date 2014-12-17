シミュレータだよ

使用方法：
1: make
2: ./simulator "実行ファイル" [option]


オプション一覧：
-p | --printinfo
終了時にレジスタの値と命令実行数を吐く

-c | --countop
終了時に命令実行数と各命令毎の実行数を吐く

-n | --nativeFPU
FPUをnative(cの方)で実行する。
-n=[option]とすることで、各命令をFPU実行にできる。
  options: asmdqfinvel
  a:add, s:sub, m:mul, d:div, q:sqrt,
  f:f2i, i:i2f, n:neg, v:inv, e:bfeq, l:bflt
  ex. -n=fiv とするとf2i,i2f,finv「以外」がnative

-d | --debug (=file)
デバッグモード。
-d=fileのようにすることで入力をfileから取る。
コマンドはヘルプ(debugger.txt)参照

-a | --disassembl
./disassembled.txtにアセンブリ化したのを吐く。
デフォルトはそのまま終了、オプションに何か（なんでも）をつけると実行を続ける。(ex. -a=*)

-r | regdump
ALU、FPU、load命令について、出力先レジスタと演算結果を吐く
出力先のデフォルトはstderr、オプションで指定可能(ex. -r=hoge.txt)

・--prとか-prとか--pとかも認識する（これは全てprintinfoになる）。
・-cpdなどとまとめられる（各オプション1文字）。


更新履歴：
12/18/01:00/fix disass
-dis-assemblオプションをfix。

12/16/15:00/ regdump
reg_dumpを実装。

12/02/14:00/disassembler
option disassemblを実装
それに伴ってprint_opが仕様変更されてる

12/01/1:30/some fix
load/storeのレジスタ型周りのバグとり

11/17/17:40/native fix
optionで個別命令をnot nativeにできるように
元の方保存を消去。

11/16/2:30/fix PC
PCの変更方法を刷新。

11/13/09:40/debug input
デバッグ時に読み込み元をstdioから変更できるようにした。
オプションが若干便利に（-cpdとかできる）。
  ・getoptはすごい

11/13/07:30/getopt
オプション判定にgetoptを使うように変更。

11/13/06:00/output to stderr
諸々の出力をstderrに変更。

11/11/14:30/native fpu fix
nativeFPUのbugfix
fpu導入時のMakefileにバグがあったようなのでfix

11/11/10:30/ feq&flt
wag氏のfeq,fltに対応(多分)

11/10/17:30/ correspond new fpu
新版fpuに対応
&some fix

11/8/2:00/
レジスタの保管方法を大幅に変更。
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
