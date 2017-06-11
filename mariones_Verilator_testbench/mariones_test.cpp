// -------------------------------------------------------------------------
// mariones_test.cpp
// 
// Developed by Hiroki Nakahara
// 28, July, 2015
// 
// Verilator向けにVivado HLSで生成したHWをサイクルベースシミュレーションするための記述
// 実行方法:
// 1. Vivado HLSで生成したVerilogファイル一式を作業用ディレクトリに持ってくる.
// 2. このファイルにシミュレーション内容を記述する
// 3. $verilator --cc -Wno-lint mariones_top.v --exe mariones_test.cpp
//  (-Wno-lintをつけないと大規模回路はシミュが止まる。おまじないと思っておけばよい)
// (トップモジュールはmariones_top.vとする)
// 4. obj_dirが生成されるので、そのディレクトリに移動
// (私は別の端末を開いて実行専用にしています。だっていちいちVerilatorを呼ぶのが面倒…)
// 5. $make -j -f Vmariones_top.mk Vmariones_top
// としてメイク。ここではシミュレーション実行コードをVmariones_topとした
// 6. シミュレーション実行
//  $./Vmariones_top
//
// 注意：Vivado HLSでメモリを生成したときは, *.datファイルをここにコピーしておく必要あり！
//
// TIPS: Vivad HLSが生成するヘッダファイル(ここではVmariones.h)をみると
//       サブモジュールのポインタが定義されているので、シミュレーション結果を表示可能
// -------------------------------------------------------------------------
#include <iostream>        // C++用のヘッダ
#include <verilated.h>     // Verilatorの共通ヘッダ
#include "Vmariones_top.h" // Verilatorが生成するヘッダファイル, 'V'をトップモジュール名に付ける

// 現在のシミュレーション時刻
unsigned int main_time = 0;

int main(int argc, char** argv) {
    // Verilator引数処理
    Verilated::commandArgs(argc, argv);

    // シミュレーション対象のインスタンス化
    Vmariones_top *top = new Vmariones_top();

    // NES本体の初期信号設定
    top->ap_clk   = 0;
    top->ap_rst   = 0;
    top->ap_start = 0;
    top->pad0     = 0x00; // 1P用コントローラ (R,L,D,U,Select,Start,B,A)だったような…
    top->pad1     = 0x00; // 2P用コントローラ

    int n_frame = 0;

    FILE *fp;
    char fname[256];
    bool file_open = false;
	
    // NESはソフトリセットで開始する
    top->reset = 1;

    // --------------------------------------------------------------
    // シミュレーションメインループ
    // クロックをトグルさせて、リセットをかけてから信号をアサートするのは
    // Verilogのシミュレーションと基本的に同じ
    // C++/Cの構文やライブラリを呼べるのがVerilatorのメリットの一つ
    // あと、サイクルベースシミュレーションなのでRTLシミュレーションと比較して１～２桁高速
    // --------------------------------------------------------------
    while (!Verilated::gotFinish()) {
        
        //  クロックをトグルさせる
        top->ap_clk = !top->ap_clk;

        // ハードウェア(FPGA)リセットやNESの起動信号のアサート処理
        if (main_time < 10){
            top->ap_rst = 1;
        } else if( main_time < 50){
            top->ap_rst = 0;
        } else {
            if( top->ap_idle == 1 && top->ap_return == 0){
               top->ap_start = 1;
            }
        }
        
        // システム(NES)のソフトリセットを解除, この時刻からシミュレーションが開始される
        if( main_time >= 500){
            top->reset = 0;
        }
        
        // シミュレーション実行
        top->eval();

        // 規定フレーム数を超えていたらシミュレーション終了
        // これでもCorei7で３０分かかったような…
		if( n_frame == 70)
		  	break;
		

        // シミュレーションの時間をインクリメントする
        main_time++;

        
        // 立ち上がりエッジかつ、1フレーム以降の状態(初期化を除く)を出力
        if( top->ap_return >= 1 && top->ap_clk == 1){
            printf("time=%8d FRM=%2d ", main_time, n_frame);
        
            printf("clk=%x rst=%x adr=%4x d0=%8x ",
        	   top->ap_clk, top->ap_rst, top->bmp_address0, top->bmp_d0
            );
            
            printf("st=%1x done=%1x idle=%1x ready=%1x ",
                top->ap_start, top->ap_done, top->ap_idle, top->ap_ready
            );
            

            printf("X=%3d,Y=%3d ", (top->bmp_address0 & 0xFF), ((top->bmp_address0 >> 8) & 0xFF));
            printf("WE=%x ", top->bmp_we0);
        
            printf("PC=%4X A=%2X X=%2X Y=%2X S=%2X ",
        	   top->v__DOT__reg_pc, top->v__DOT__reg_a, top->v__DOT__reg_x, 
        	   top->v__DOT__reg_y, top->v__DOT__reg_s);

            printf("nvbdizc=%x,%x,%x,%x,%x,%x,%x",
                top->v__DOT__n_flag, top->v__DOT__v_flag, 
                top->v__DOT__b_flag, top->v__DOT__d_flag, 
                top->v__DOT__i_flag, top->v__DOT__z_flag,
                top->v__DOT__c_flag
                );

            printf("\n");
        }


        // フレーム数を更新し, 規定フレーム以上になったらBMPデータを表示するためのファイルオープン
        // スーパーマリオは確か40フレームしないとタイトル画面が出てこなかったから…
        // (真っ黒な画面を出力してもデバッグにならないし)
        if( top->ap_return != n_frame && top->ap_clk == 1){
            n_frame++;
            printf("---------- frame=%d ------------- \n", n_frame);

            if( n_frame > 50 && file_open == true){
                fclose( fp);

                file_open = false;
            }

            if( n_frame >= 50 && file_open == false){

                sprintf( fname, "frame_%d.txt", n_frame);
                fp = fopen( fname, "w");
                if( fp == NULL){
                    fprintf( stderr, "CAN'T OPEN %s\n", fname);
                    exit(-1);
                }

                file_open = true;
            }
        }

        // (X,Y)における画素(確かRGB各8ビットだったような…)をファイル出力
        // 別のプログラム(dat2bmp)でBMP画像に変換する
        if( top->ap_clk == 1 && top->bmp_we0 == 1 && n_frame >= 50 && file_open == true){
            fprintf( fp, "X= %d Y= %d %X\n", 
                        (top->bmp_address0 & 0xFF), ((top->bmp_address0 >> 8) & 0xFF),
                        top->bmp_d0);
        }

    }

    // ファイルをオープンしていたらクローズ
    if( file_open == true)
        fclose( fp);

    // シミュレーションのインスタンスを閉じる
    top->final();
}
// ----------------------------------------------------------------------------
//                                END OF FILE
// ----------------------------------------------------------------------------
