これは何 ----------------------------------------------------------------
NESエミュレータのHDLソースコード一式. これにVivado HLSで合成したコードを取り込んで
Vivadoで合成すると完成.

Developed by Hiroki Nakahara
 31, July, 2015

ライセンス ------------------------------------------------------
このコード一式はみきゃんウェアです。ゆるキャラグランプリ２０１５で
みきゃんに投票して頂ければ自由に配布・改変・公開してかまいません。

同梱物 -----------------------------------------------------------------

Nexys4DDR_Master.xdc … 制約ファイル. Nexys4 DDR 専用なので, デバイスごとに
　　　　　　　　　　　　　　　　　　　このファイルを差し換えてください。
NES.v … NESトップファイル. NESがトップモジュールになります。
vga640x480.v … VGAコントローラ. Digilentから拝借して追加しました。

環境 -------------------------------------------------------------------
Vivado2015.1@Win8.1, Nexys4 DDR をターゲットにしています。
Zybo, Zedboard, Nexys4辺りは動くんじゃないでしょうか（他人事…）

実行方法 ---------------------------------------------------------------
1. Vivado HLSから合成したVerilog HDL一式をここに持ってくる。メモリ初期ファイル(*.dat)も
　忘れずに！

2. Vivado を起動してプロジェクト作成.

3. バッファメモリをIP Catalogから生成する. VivadoのFlow Navigatorから IP Catalogを
　選択する. Vivado Repository->Memories & Storage Elements -> RANs & ROMs
          -> Block Memory Generator をダブルクリックして起動.

 Customize IPのBasicタブを選択し, Memory Typeを Simple Dual Port RAMに設定
               Port A Optionsタブを選択し, Port A Width = 12,
                                        Port A Depth = 131072 と入力してOKをクリック。
 Project Manager -> Sourcesにメモリが追加されればおｋ

 4. Generate Bitstream をクリックして２０分ほど待つ。
 　　Critical Warningが出ても気にしない…

 5. Nexys4 DDRにビットストリームを流し込んで終了。
 NES.v を見ればわかるのですが、１プレイヤだけスイッチを割り付けています。
 ソフトウェアリセット SW[1] をon->off としてリセットをかけてください。

問い合わせ先 ------------------------------------------------------
Hiroki Nakahara, @oboe7man (twitter)
忙しいので対応できないかもしれません、すみません…
