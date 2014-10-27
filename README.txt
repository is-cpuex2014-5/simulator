シミュレータだよ


使用方法：
1: make
2: ./simulator [実行ファイル]

・エンディアン変更に伴い既存のtestsは動かない


更新履歴：
10/17/18:00/moromoro
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
