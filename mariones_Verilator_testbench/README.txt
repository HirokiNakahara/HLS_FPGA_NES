これは何 ----------------------------------------------------------------
Vivado HLSで生成したHWをVerilatorを使ってサイクルベースシミュレーションするための
 テストベンチ一式

Developed by Hiroki Nakahara
 31, July, 2015

ライセンス ------------------------------------------------------
このコード一式はみきゃんウェアです。ゆるキャラグランプリ２０１５で
みきゃんに投票して頂ければ自由に配布・改変・公開してかまいません。

同梱物 -----------------------------------------------------------------

mariones_test.cpp … テストベンチ。でもCで記述しています。。C++を書く気力がなかった
dat2bmp.c … シミュレーション後に出力される画像データ(frame_??.txt)を
            BMP画像に変換するツール.

環境 -------------------------------------------------------------------
Ubuntsu14.04 LTS上で確認しています。
Verilatorは $sudo apt-get install verilator で一発でインストールできる（はず）

実行方法 ---------------------------------------------------------------
1. Vivado HLSで生成したVerilogファイル一式を作業用ディレクトリに持ってくる.

2. mariones_test.cppにシミュレーション内容を記述する.
  デフォルトでも60フレームほどシミュレーションして画像データ(.txt)を出力してくれます。

3. $verilator --cc -Wno-lint mariones_top.v --exe mariones_test.cpp
  (-Wno-lintをつけないと大規模回路はシミュが止まる。おまじないと思っておけばよい)
  (トップモジュールはmariones_top.vとする)

4. obj_dirが生成されるので、そのディレクトリに移動
 (私は別の端末を開いて実行専用にしています。だっていちいちVerilatorを呼ぶのが面倒…)

5. $make -j -f Vmariones_top.mk Vmariones_top
  としてメイク。ここではシミュレーション実行コードをVmariones_topとした

6. シミュレーション実行
   $./Vmariones_top

  同梱の dat2bmp.c をgccでコンパイルしてバイナリを作り,

  $ ./dat2bmp frame_??.txt としてBMP画像を出力します.

注意：Vivado HLSでメモリを生成したときは, *.datファイルをobj_dirにコピーしておく必要あり！

TIPS: Verilatorが生成するヘッダファイル(ここではVmariones_top.h)をみると
      サブモジュールのポインタが定義されているので、シミュレーション結果を表示可能

問い合わせ先 ------------------------------------------------------
Hiroki Nakahara, @oboe7man (twitter)
忙しいので対応できないかもしれません、すみません…
