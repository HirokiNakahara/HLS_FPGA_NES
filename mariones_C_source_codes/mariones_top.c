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

// external functions -----------------------------------------
// mbc (メモリアクセス部)
unsigned char mbc_read( int adr);
void mbc_write( int adr, unsigned char dat);
unsigned char mbc_read_chr_rom( int adr);
void mbc_write_chr_rom( int adr, int dat);

// レジスタ関連
void regs_set_vblank( int b, int nmi);
void regs_start_frame();
void regs_start_scanline();
void regs_end_scanline();
int regs_draw_enabled();
void regs_set_input( int *dat);
unsigned char regs_read( int adr);
void regs_write( int adr, unsigned char dat);
unsigned char regs_read_2007();
void regs_write_2007( unsigned char dat);

// cpu関連
void cpu_set_nmi( int b);
void cpu_set_irq( int b);
void cpu_set_reset( int b);
unsigned char cpu_read8( int adr);
int cpu_read16( int adr);
void cpu_write8( int adr, unsigned char dat);
void cpu_write16( int adr, int dat);
void cpu_exec( int clk);
void cpu_exec_irq( int it);
int cpu_get_master_clock();
double cpu_get_frequency();
void cpu_log();

//　ppu（描画ユニット）関連
void ppu_render( int line, unsigned char *bmp);
void ppu_render_bg(int line, unsigned char *buf);
int ppu_render_spr(int line, unsigned char *buf);
void ppu_sprite_check( int line);

unsigned char r_name_table( int adr);
void w_name_table( int adr, unsigned char dat);

// NESリセット関数
void reset_nes();

// ------------------------------------------------------------------
// external variables
// booleanとコメントがついているのはフラグ用変数
// ------------------------------------------------------------------
// Variables for ROM --------------------------------------
extern int prg_page_cnt;
extern int chr_page_cnt;
enum mirror_type{ HORIZONTAL, VERTICAL};

extern int sram_enable, trainer_enable, four_screen; // boolean
extern int mapper;
	
// variables for MBC --------------------------------------
extern unsigned char ram[ 0x800 + 1];
extern int is_vram;
	
// variables for registers --------------------------------
extern int nmi_enable; // boolean
extern int sprite_size; // boolean
extern int bg_pat_adr, sprite_pat_adr; // boolean
extern int ppu_adr_incr; // boolean
extern int name_tbl_adr;
	
extern int bg_color;
extern int sprite_visible, bg_visible; // boolean
extern int sprite_clip, bg_clip; // boolean
extern int color_display; // boolean
	
extern int is_vblank; // boolean
extern int sprite0_occur, sprite_over; // boolean
extern int vram_write_flag; // boolean
	
extern unsigned char sprram_adr;
extern unsigned int ppu_adr_t, ppu_adr_v, ppu_adr_x;
extern int ppu_adr_toggle;  // boolean
extern unsigned char ppu_read_buf;
extern int joypad_strobe;  // boolean
extern int joypad_read_pos[2], joypad_sign[2];
extern int frame_irq;
	
extern int pad_dat[2][8]; // boolean
	
// variables for CPU --------------------------------------
extern unsigned char reg_a, reg_x,reg_y, reg_s;
extern unsigned int reg_pc;
extern unsigned char c_flag, z_flag, i_flag, d_flag, b_flag, v_flag, n_flag;
	
enum irq_type{ NMI, IRQ, RESET};
	
extern int rest;
extern unsigned int mclock; // 64 bit??
extern int nmi_line, irq_line, reset_line; // boolean
	
extern unsigned char opc;
	
// variables for PPU --------------------------------------
extern unsigned char sprram[0x100];
extern unsigned char name_table[0x1000];
extern unsigned char palette[0x20];
	
extern int nes_palette_24[0x40];
extern int nes_palette_16[0x40];

// for debug
int frame_number;
	
// #######################################################################
// 
// Main Function for NES emulator
// 
// #######################################################################
int mariones_top( 
	int reset,
	unsigned char bmp[256*3*240],
	int pad[2][8]
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
	} else {
		// ########################################################
		// Execute NES
		// ########################################################
		
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
		for( i = 0; i < 240; i++){
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
		if( frame_irq & 0xC0 == 0)
			cpu_set_irq( 1);
		
		// ------------------------------------------------------------------
		// CPU処理エミュレーション
		// NESは描画終了時にMNI割り込みを発生させて, 描画ビームがX=0,Y=0に戻る間に
		// 処理（キャラの移動とか得点計算とか）を行う
		// 逆にPPUはだいたいヒマね...
		// ------------------------------------------------------------------
		for( i = 240; i < 262; i++){
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
	// mariones_top() 終了 ------------------------------------------------------------
}
// ------------------------------------------------------------------------------------
//                                     END OF PROGRAM
// ------------------------------------------------------------------------------------


