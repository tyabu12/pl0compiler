# PL/0' Compiler

中田育男先生の教科書に載っているコードに多少の改変を施したものです。
オリジナルコードは以下のURLにて配布されています。

http://www.k.hosei.ac.jp/~nakata/oCompiler/PL0compiler/prog_proc1.html

## コンパイル方法

以下を実行することにより、コマンド`pl0c`と`pl0e`が生成される。
~~~
$ make
~~~

## 使い方

コマンドをパスが通ったディレクトリにコマンドを置く。

### コンパイル

PL/0'プログラム`example.pl0`をコンパイル:
~~~
$ pl0c example.pl0
~~~
バイトコードを格納したファイル`a.out`が生成される。

### 実行

機械語ファイル`a.out`を仮想機械で実行:
~~~
$ pl0e a.out
~~~

## 参考文献

中田育男:「コンパイラ」オーム社 (1995).
中田育男:「コンパイラの構成と最適化」, 第2版, 朝倉書店 (2009).
ウィキペディア日本語版:「PL/0」, https://ja.wikipedia.org/wiki/PL/0.
N. Wirth: "Algorithms + Data Structures = Programs", Prentice-Hall (1975).
