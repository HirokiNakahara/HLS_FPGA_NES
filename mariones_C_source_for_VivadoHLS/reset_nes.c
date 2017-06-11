// -------------------------------------------------------------------------
// reset_nes.c
// 
// Developed by Hiroki Nakahara
// 31, July, 2015
// 
// 各モジュールのレジスタ・メモリの初期化を行う
// C++/Javaだとコンストラクタが使えるので、こういうルーチンいらないんだよな…
// -------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mariones.h"

// ########################################################
// Reset NES
// ########################################################
void reset_nes()
{
	// reset mbc ----------------------------------------------
	int i;
	for( i = 0; i < 0x800; i++) ram[i] = 0x0;
	
	// reset ppu ----------------------------------------------
	for( i = 0; i < 0x100; i++) sprram[i] = 0x0;
	for( i = 0; i < 0x1000; i++) name_table[i] = 0x0;
	for( i = 0; i < 0x20; i++) palette[i] = 0x0;

	// reset registers ----------------------------------------
	nmi_enable = 0;
	sprite_size = 0;
	bg_pat_adr = sprite_pat_adr = 0;
	ppu_adr_incr = 0;
	name_tbl_adr = 0;
	
	bg_color = 0;
	sprite_visible = bg_visible = 0;
	sprite_clip = bg_clip = 0;
	color_display = 0;
	
	is_vblank = 0;
	sprite0_occur = sprite_over = 0;
	vram_write_flag = 0;
	
	sprram_adr = 0;
	ppu_adr_t = ppu_adr_v = ppu_adr_x = 0;
	ppu_adr_toggle= 0;
	ppu_read_buf = 0;
	
	joypad_strobe = 0;
	joypad_read_pos[0] = joypad_read_pos[1] = 0;
	joypad_sign[0] = 0x1, joypad_sign[1] = 0x2;
	
	frame_irq = 0xFF;
	
	// reset cpu ----------------------------------------------	
	reg_a = 0x00;
	reg_x = 0x00;
	reg_y = 0x00;
	reg_s = 0xFF;
	
	n_flag = v_flag = z_flag = c_flag = 0;
	b_flag = 1;
	d_flag = 0;
	i_flag = 1;
	
	rest   = 0;
	mclock = 0;
	nmi_line = irq_line = reset_line = 0;
	
	// PCの初期値はゲーム毎に違うので、初期値を一概に決定できない
	reg_pc = cpu_read16( 0xFFFC);

}
// ------------------------------------------------------------------------------------
//                                     END OF PROGRAM
// ------------------------------------------------------------------------------------
