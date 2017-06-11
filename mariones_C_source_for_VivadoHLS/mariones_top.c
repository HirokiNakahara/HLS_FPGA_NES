// -------------------------------------------------------------------------
// mariones_top.c
// 
// Developed by Hiroki Nakahara
// 31, July, 2015
// 
// NES本体. この関数以下をVivado HLSで高位合成対象とする.
// エミュレーションを行うのは1フレームのみ. 外部変数でレジスタの値を保持しておいて
// main.c　から何度も呼び出す構造.
// -------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 共通ヘッダ. 外部変数もここに
#include "mariones.h"

// 任意精度型を使うためのヘッダ
#include "ap_cint.h"

// external functions -----------------------------------------

// レジスタ関連
void regs_set_vblank( int b, int nmi);
void regs_start_frame();
void regs_start_scanline();
void regs_end_scanline();

// cpu関連
void cpu_set_irq( int b);
void cpu_exec( int clk);
void cpu_exec_irq( int it);

//　ppu（描画ユニット）関連
void ppu_render( int line, uint12 *bmp);
void ppu_sprite_check( int line);

// NESリセット関数
void reset_nes();

// ------------------------------------------------------------------
// external variables
// booleanとコメントがついているのはフラグ用変数
// ------------------------------------------------------------------
// Variables for ROM --------------------------------------
enum mirror_type{ HORIZONTAL, VERTICAL};
	
// variables for registers --------------------------------
extern int is_vblank; // boolean
extern int frame_irq;	
extern int pad_dat[2][8]; // boolean
	
// variables for CPU --------------------------------------
enum irq_type{ NMI, IRQ, RESET};

// for debug
static int frame_num = 0;
	
// #######################################################################
// 
// Main Function for NES emulator
// 
// #######################################################################
int mariones_top(
	uint1 reset,
	uint12 bmp[256*240],
	unsigned char pad0,
	unsigned char pad1
)
{
	// ########################################################
	// Definition for Variables
	// ########################################################
	// local variables
	int i, j;
	int cnt;
	
	if( reset == 1){
		// ソフトウェアリセットが要求されたとき, リセットを行う
		reset_nes();
		frame_num = 0;
	} else {
		// ########################################################
		// Execute NES
		// ########################################################
		
		/* ゲームパッド読み込み                 */
		PAD0_READ: for( i = 0; i < 8; i++)     // Vivado HLSで解析しやすくするために
			pad_dat[0][i] = (pad0 >> i) & 0x1; // ループにはラベルを付けておくとよい
		PAD1_READ: for( i = 0; i < 8; i++)
			pad_dat[1][i] = (pad1 >> i) & 0x1;

		// MNI割り込み実行
		exec_MNI = 0;			
		regs_set_vblank( 0, 1);
		
		if( exec_MNI == 1){
			// エッジセンシティブな割り込み
  			if ( !nmi_line && is_vblank)
    			cpu_exec_irq( NMI);
  			nmi_line = is_vblank;
		}
		exec_MNI = 0;
		
		// フレーム描画開始
		regs_start_frame();
		
		// ----------------------------------------------------------
		// Y=0からY=239ライン描画エミュレーション
		// スプライトのチェックも行う
		// bmp[]に24ビット(RGB各8ビット)に画面が格納される
		// この間はCPUはお休み...
		// ----------------------------------------------------------
		LOOP_RENDER: for( i = 0; i < 240; i++){
		//for( i = 0; i < 20; i++){
			//if( mapper) mapper_hblank( i); // this version does not support
			
			// for debug
			frame_number = cnt;
			
			// 1ラインを描画
			regs_start_scanline();
			ppu_render( i, bmp);
			ppu_sprite_check( i);
			
			/*
			if( cnt == 5)
			printf(" i=%d PC=%X OC=%X A=%X X=%X Y=%X S=%X n=%x v=%x b=%x d=%x i=%x z=%x c=%x\n",
				i, reg_pc, opc, reg_a, reg_x, reg_y, reg_s, n_flag, 
				v_flag, b_flag, d_flag, i_flag, z_flag, c_flag);
			*/
			
			// Super Mario Bros.だけの高速化方法, 描画時はcpuがお休みなのでスキップ
			// ただし, ここではスキップしないけど...
			/*
			if( reg_pc == 0x8057){
				
			} else {
				cpu_exec( 114);
			}
			*/
			cpu_exec( 114);
			if( exec_MNI == 1){
				//printf("\n0:\nexec_MNI=%d\n\n", exec_MNI);
				
				// エッジセンシティブな割り込み
				if ( !nmi_line && is_vblank)
					cpu_exec_irq( NMI);
				nmi_line = is_vblank;
				exec_MNI = 0;
			}

			//　ライン描画終わり
			regs_end_scanline( );
		}
		// 画面描画終わり ------------------------------------------------------
		//exit(0);
		

		// 	フレーム割り込みセット
		if( (frame_irq & 0xC0) == 0)
			cpu_set_irq( 1);
		
		// ------------------------------------------------------------------
		// CPU処理エミュレーション
		// NESは描画終了時にMNI割り込みを発生させて, 描画ビームがX=0,Y=0に戻る間に
		// 処理（キャラの移動とか得点計算とか）を行う
		// 逆にPPUはだいたいヒマね...
		// ------------------------------------------------------------------
		LOOP_CPU_EXEC: for( i = 240; i < 262; i++){
			//if( mapper) mapper_hblank( i); // this version does not support
			
			if( i == 241){
				regs_set_vblank( 1, 0);
				if( exec_MNI == 1){
					//printf("\n1:\nexec_MNI=%d\n\n", exec_MNI);				
					// エッジセンシティブな割り込み
					if ( !nmi_line && is_vblank)
						cpu_exec_irq( NMI);
					nmi_line = is_vblank;
					exec_MNI = 0;
				}
				
				// とりあえず1命令だけ実行
				cpu_exec( 0);
				regs_set_vblank( 1, 1);
				if( exec_MNI == 1){
					if ( !nmi_line && is_vblank)
						cpu_exec_irq( NMI);
					nmi_line = is_vblank;
					exec_MNI = 0;
				}
				
				// 本命。CPUのエミュレーション実行
				cpu_exec( 114);
				if( exec_MNI == 1){
					if ( !nmi_line && is_vblank)
						cpu_exec_irq( NMI);
					nmi_line = is_vblank;
					exec_MNI = 0;
				}

			} else {
				cpu_exec( 114);
				if( exec_MNI == 1){
					if ( !nmi_line && is_vblank)
						cpu_exec_irq( NMI);
					nmi_line = is_vblank;
					exec_MNI = 0;
				}

			}
			
			/*
			if( cnt == 5){
			printf(" i=%d PC=%X OC=%X A=%X X=%X Y=%X S=%X n=%x v=%x b=%x d=%x i=%x z=%x c=%x\n",
				i, reg_pc, opc, reg_a, reg_x, reg_y, reg_s, n_flag, v_flag, b_flag, d_flag, i_flag, z_flag, c_flag);
			}
			*/
				
		}
		// CPU処理エミュレーション終了 ------------------------------------------------------

		//printf("cnt=%X PC=%X OC=%X A=%X X=%X Y=%X S=%X n=%x v=%x b=%x d=%x i=%x z=%x c=%x\n",
		//		cnt, reg_pc, opc, reg_a, reg_x, reg_y, reg_s, n_flag, v_flag, b_flag, d_flag, i_flag, z_flag, c_flag);
	}

	frame_num++;
	// mariones_top() 終了 ------------------------------------------------------------

	return frame_num; /* 現在処理中のフレーム番号を返す */
}
// ------------------------------------------------------------------------------------
//                                     END OF PROGRAM
// ------------------------------------------------------------------------------------


