# HLS_FPGA_NES
An FPGA NES emulator designed by a high level synthesis (HLS)

## これは何  
NESエミュレータのソースコード一式. Vivado HLS2015.1で合成しました.

[FPGAエクストリームコンピューティングで発表した資料](https://www.slideshare.net/HirokiNakahara1/fps530000-51179724)

Developed by Hiroki Nakahara
 31, July, 2015

## ライセンス
~~このコード一式はみきゃんウェアです。ゆるキャラグランプリ２０１５で
みきゃんに投票して頂ければ自由に配布・改変・公開してかまいません。~~
ゆるキャラグランプリははるか昔に終わったのでMITライセンスへ。

## 同梱物  

mariones_C_source_codes         … 一応, 高位合成可能なCソースコード一式.
mariones_C_source_for_VivadoHLS … CソースコードをVivadoHLS向けに修正したもの.
mariones_Verilator_testbench    … Vivado HLSで合成したRTLを検証するための  
                                   Verilatorテストベンチ.
mariones_VerilogHDLs            … Vivado HLSで合成したRTLと一緒にNESを合成するための  
                                   HDL一式. グルーロジックとVGAコントローラが入っています.

## 環境  
Vivado2015.1@Win8.1, Nexys4 DDR をターゲットにしています。  
Zybo, Zedboard, Nexys4辺りは動くんじゃないでしょうか（他人事…）

CソースコードはUbuntu14.04LTS, Cygwin 64bit@Win8.1上のgccでコンパイル可能.  
VerilatorはUbuntu14.04LTSで確認.  
HDLはVivado2015.1で合成, Nexys4 DDRで動作.

## 注意  
**ファミコンカートリッジイメージは自前で用意すること！**
**ネットからダウンロードしたら違法**だよ！**ネットからダウンロードしたら違法**だよ！
（大事なことなので２回言いました）

私はArduinoを使って吸出し機を作りました.

## 実行方法  
VivadoHLS向けに修正したCコード(mariones_C_source_for_VivadoHLS)を  
Vivado HLSで高位合成してHDLを作成後, 全体のロジック(mariones_VerilogHDLs)に  
コピーしてVivadoでビットストリーム生成でおｋ

検証したければVerilatorを使ってください.  
オリジナルのCコードを見たい時は(mariones_C_source_codes)を見てください.

## 問い合わせ先  
Hiroki Nakahara, ~~@oboe7man (twitter)~~
忙しいので対応できないかもしれません、すみません…
てか、忙しすぎてtwitterやめました…
