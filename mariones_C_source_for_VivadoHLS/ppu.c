// -------------------------------------------------------------------------
// PPU.c
// 
// Developed by Hiroki Nakahara
// 31, July, 2015
// 
// PPU(描画ユニット)のエミュレーションを行う
// -------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "mariones.h"

/* 任意精度型を使うためのヘッダ */
#include "ap_cint.h"

void ppu_render( int line, uint12 *bmp); // 描画バッファメモリを節約するために12ビット指定
void ppu_render_bg(int line, unsigned char *buf);
int ppu_render_spr(int line, unsigned char *buf);
void ppu_sprite_check( int line);

unsigned char r_name_table( int adr);
void w_name_table( int adr, unsigned char dat);

// パレットデータ, (Blue, Green, Red)各8bit, 24bitカラー
// Vivado HLSで合成するときは上位4bit x 3 = 12bitで
// よく考えたら、パレットって64個しかないから、パレット番号を外部バッファに入れておいて
// VGAモニタに出力するとき、このテーブルを引くようにすればメモリを半分にできるんだった…orz

uint4 pal_dat[0x40][3]={
    {0x7,0x7,0x7}, {0x2,0x1,0x8}, {0x0,0x0,0xA}, {0x4,0x0,0x9},
    {0x8,0x0,0x7}, {0xA,0x0,0x1}, {0xA,0x0,0x0}, {0x7,0x0,0x0},
    {0x4,0x2,0x0}, {0x0,0x4,0x0}, {0x0,0x5,0x0}, {0x0,0x3,0x1},
    {0x1,0x3,0x5}, {0x0,0x0,0x0}, {0x0,0x0,0x0}, {0x0,0x0,0x0},

    {0xB,0xB,0xB}, {0x0,0x7,0xE}, {0x2,0x3,0xE}, {0x8,0x0,0xF},
    {0xB,0x0,0xB}, {0xE,0x0,0x5}, {0xD,0x2,0x0}, {0xC,0x4,0x0},
    {0x8,0x7,0x0}, {0x0,0x9,0x0}, {0x0,0xA,0x0}, {0x0,0x9,0x3},
    {0x0,0x8,0x8}, {0x1,0x1,0x1}, {0x0,0x0,0x0}, {0x0,0x0,0x0},

    {0xF,0xF,0xF}, {0x3,0xB,0xF}, {0x5,0x9,0xF}, {0xA,0x8,0xF},
    {0xF,0x7,0xF}, {0xF,0x7,0xB}, {0xF,0x7,0x6}, {0xF,0x9,0x3},
    {0xF,0xB,0x3}, {0x8,0xD,0x1}, {0x4,0xD,0x4}, {0x5,0xF,0x9},
    {0x0,0xE,0xD}, {0x6,0x6,0x6}, {0x0,0x0,0x0}, {0x0,0x0,0x0},

    {0xF,0xF,0xF}, {0xA,0xE,0xF}, {0xC,0xD,0xF}, {0xD,0xC,0xF},
    {0xF,0xC,0xF}, {0xF,0xC,0xD}, {0xF,0xB,0xB}, {0xF,0xD,0xA},
    {0xF,0xE,0xA}, {0xE,0xF,0xA}, {0xA,0xF,0xB}, {0xB,0xF,0xC},
    {0x9,0xF,0xF}, {0xD,0xD,0xD}, {0x1,0x1,0x1}, {0x1,0x1,0x1}
    };

/* 一応, 元のパレットデータ残しておきますね...
int pal_dat[0x40][3]={
    {0x75,0x75,0x75}, {0x27,0x1B,0x8F}, {0x00,0x00,0xAB}, {0x47,0x00,0x9F},
    {0x8F,0x00,0x77}, {0xAB,0x00,0x13}, {0xA7,0x00,0x00}, {0x7F,0x0B,0x00},
    {0x43,0x2F,0x00}, {0x00,0x47,0x00}, {0x00,0x51,0x00}, {0x00,0x3F,0x17},
    {0x1B,0x3F,0x5F}, {0x00,0x00,0x00}, {0x05,0x05,0x05}, {0x05,0x05,0x05},
    
    {0xBC,0xBC,0xBC}, {0x00,0x73,0xEF}, {0x23,0x3B,0xEF}, {0x83,0x00,0xF3},
    {0xBF,0x00,0xBF}, {0xE7,0x00,0x5B}, {0xDB,0x2B,0x00}, {0xCB,0x4F,0x0F},
    {0x8B,0x73,0x00}, {0x00,0x97,0x00}, {0x00,0xAB,0x00}, {0x00,0x93,0x3B},
    {0x00,0x83,0x8B}, {0x11,0x11,0x11}, {0x09,0x09,0x09}, {0x09,0x09,0x09},
    
    {0xFF,0xFF,0xFF}, {0x3F,0xBF,0xFF}, {0x5F,0x97,0xFF}, {0xA7,0x8B,0xFD},
    {0xF7,0x7B,0xFF}, {0xFF,0x77,0xB7}, {0xFF,0x77,0x63}, {0xFF,0x9B,0x3B},
    {0xF3,0xBF,0x3F}, {0x83,0xD3,0x13}, {0x4F,0xDF,0x4B}, {0x58,0xF8,0x98},
    {0x00,0xEB,0xDB}, {0x66,0x66,0x66}, {0x0D,0x0D,0x0D}, {0x0D,0x0D,0x0D},
    
    {0xFF,0xFF,0xFF}, {0xAB,0xE7,0xFF}, {0xC7,0xD7,0xFF}, {0xD7,0xCB,0xFF},
    {0xFF,0xC7,0xFF}, {0xFF,0xC7,0xDB}, {0xFF,0xBF,0xB3}, {0xFF,0xDB,0xAB},
    {0xFF,0xE7,0xA3}, {0xE3,0xFF,0xA3}, {0xAB,0xF3,0xBF}, {0xB3,0xFF,0xCF},
    {0x9F,0xFF,0xF3}, {0xDD,0xDD,0xDD}, {0x11,0x11,0x11}, {0x11,0x11,0x11}
  	};
*/	
// ########################################################
// Emulation for PPU
// ########################################################

// １行分の描画バッファ
// 背景とスプライトを一時的に書き込むために使用 
unsigned char buf[ 256 + 16]; 

// -----------------------------------------------------------------
// PPUレンダラ、各関数を呼び出して描画を行う
// (各描画には依存関係がないので、並列化しやすい)
// -----------------------------------------------------------------
void ppu_render( int line, uint12 *bmp)
{
	int i;
  unsigned char *p = buf + 8;
	
  // パレット初期化
  PPU_CLK_PAL: for ( i = 0; i < 256; i++)
    p[i] = palette[ 0];
	
  // 背景描画
  if ( bg_visible)
    ppu_render_bg( line, p);
  
  // スプライト描画
  if ( sprite_visible)
    ppu_render_spr( line, p);

  // １行の描画データを バッファ(bmp)に転送
	PPU_PUT_PICS: for ( i = 0; i < 256; i++){
    bmp[ line * 256 + i] = (pal_dat[ p[i] & 0x3f][2] << 8)
               | (pal_dat[ p[i] & 0x3f][1] << 4)
               | (pal_dat[ p[i] & 0x3f][0]);
	}
}

#define read_pat_table( adr) (mbc_read_chr_rom(adr))

// ------------------------------------------------------------------------
// 背景の描画
// ------------------------------------------------------------------------
void ppu_render_bg(int line, unsigned char *p_buf)
{
	int i, j;
	int x_ofs    = ppu_adr_x & 7;
  int y_ofs    = ( ppu_adr_v >> 12) & 7;
  unsigned int name_adr = ppu_adr_v & 0xfff;
  unsigned int pat_adr  = bg_pat_adr ? 0x1000 : 0x0000;
	
	
  p_buf -= x_ofs;
  PPU_RENDER_BG: for ( i = 0; i < 33; i++, p_buf += 8){
    unsigned char tile = r_name_table( 0x2000 + name_adr);
		
    unsigned char l = read_pat_table( pat_adr + tile * 16 + y_ofs);
    unsigned char u = read_pat_table( pat_adr + tile * 16 + y_ofs + 8);
		
    int tx = name_adr & 0x1f, ty = ( name_adr >> 5) & 0x1f;
    unsigned int attr_adr = ( name_adr & 0xC00) + 0x3C0 + ( (ty & ~3) << 1) + ( tx >> 2);
    int aofs = (( ty & 2) == 0 ? 0 : 4) + (( tx & 2) == 0 ? 0 : 2);
    int attr = (( r_name_table( 0x2000 + attr_adr) >> aofs ) & 3) << 2;
		
		
    for ( j = 7; j >= 0; j--){
      int t = (( l & 1) | ( u << 1)) & 3;
      if ( t != 0) 
      	p_buf[ j] = 0x40 | palette[ t | attr];
    		
      l >>= 1, u >>= 1;
    }
		
    if (( name_adr & 0x1f) == 0x1f)
      name_adr = ( name_adr & ~0x1f) ^ 0x400;
    else
      name_adr++;
  }
}

// -----------------------------------------------------------------------
// スプライトを描画する
// -----------------------------------------------------------------------
int ppu_render_spr(int line, unsigned char *buf)
{
	int i, x;
  int spr_height = sprite_size ? 16 : 8;
  int pat_adr    = sprite_pat_adr ? 0x1000 : 0x0000;
  
  PPU_RENDER_SPR: for ( i = 0; i < 64; i++){
    int spr_y = sprram[ i * 4 + 0] + 1;
    int attr  = sprram[ i * 4 + 2];
		
    if (!(line >= spr_y && line < spr_y + spr_height))
      continue;
		
    unsigned int is_bg = (( attr >> 5) & 1) !=0; // boolean
    int y_ofs = line - spr_y;
    int tile_index = sprram[ i * 4 + 1];
    int spr_x = sprram[ i * 4 + 3];
    int upper = ( attr & 3) << 2;
		
    unsigned int h_flip = ( attr & 0x40) == 0; // boolean
    int sx = h_flip ? 7 : 0,ex = h_flip ? -1 : 8, ix = h_flip ? -1 : 1;
		
    if (( attr & 0x80) != 0)
      	y_ofs = spr_height - 1 - y_ofs;
		
    unsigned int tile_adr;
    if ( spr_height == 16)
      	tile_adr = ( tile_index & ~1) * 16 + (( tile_index & 1) * 0x1000) 
                    + ( y_ofs >=8 ? 16 : 0) + ( y_ofs & 7);
    else
      	tile_adr = pat_adr + tile_index * 16 + y_ofs;
		
    unsigned char l = read_pat_table( tile_adr);
    unsigned char u = read_pat_table( tile_adr + 8);
		
    // 背面スプライトを描画されていた場合、その上にBGがかぶさっていても、
    // その上に前面スプライトが描画されることはない。
    for ( x = sx; x != ex; x += ix){
      	int lower = ( l & 1) | (( u & 1) << 1);
      	
        if ( lower !=0 && ( buf[ spr_x + x] & 0x80) == 0){
        	if ( !is_bg || ( buf[ spr_x + x] & 0x40) == 0)
          		buf[ spr_x + x] = palette[ 0x10 | upper | lower];
        	
          buf[ spr_x + x] |= 0x80;
      	}
      	l >>= 1, u >>= 1;
    }
  }

  return 0;
}

// ---------------------------------------------------------------
// sprite 0 hit をチェックする。
// 色々な使い方があるらしい…
// ---------------------------------------------------------------
void ppu_sprite_check( int line)
{
  if (sprite_visible){
    int spr_y = sprram[ 0] + 1;
    int spr_height = sprite_size ? 16 : 8;
    int pat_adr = sprite_pat_adr ? 0x1000 : 0x0000;

    if ( line >= spr_y && line < spr_y + spr_height){
      int y_ofs = line - spr_y;
      int tile_index = sprram[ 1];
      if (( sprram[ 2] & 0x80) != 0)
        y_ofs = spr_height - 1 - y_ofs;
      unsigned int tile_adr;
      if ( spr_height == 16)
        tile_adr = ( tile_index & ~1) * 16 + (( tile_index & 1) * 0x1000) 
          + ( y_ofs >=8 ? 16 : 0) + ( y_ofs & 7);
      else
        tile_adr = pat_adr + tile_index * 16 + y_ofs;
      unsigned char l = read_pat_table( tile_adr);
      unsigned char u = read_pat_table( tile_adr + 8);
      if ( l != 0 || u != 0)
        sprite0_occur = 1; //true;
    }
  }
}

// ------------------------------------------------------------------------------
// ネームテーブルを模擬する関数
// ミラーリングに応じてネームテーブル（スプライトのパターン）の内容を読みだす
// 
// mirroring = 0
// +---------------+
// |               |
// |    hogehoge   | 
// |               |
// +---------------+
// 
// 
// mirroring = 1 (Vertical mirroring)
// 横スクロールに使われる
// +---------------+---------------+
// |          +----|---------+     |
// |    hogehoge   |     NULPO ->  |
// |          |    |         | ->  |  <- こんな感じに
// +----------|----+---------|-----+ 　　　１画面を切り出して
// |          +----|---------+ ->  | 　　　背景をスクロールしている
// |    hogehoge   |     NULPO     | 
// |               |               |
// +---------------+---------------+
//
// mirroring = 2 (Holizontal mirroring)
// 縦スクロールに使われる
// +---------------+---------------+
// |               |               |
// |    hogehoge   |   hogehoge    |
// |               |               |
// +---------------+---------------+
// |               |               |
// |      NULPO    |     NULPO     | 
// |               |               |
// +---------------+---------------+
//
// 別配布のpartition_ROM.exe (partition_ROM.cを単体でgcc -O3 とかでコンパイル)を
// 使ってミラーリング(mirroring)番号を調べて、該当する部分のコメントを外してください
// 
// ------------------------------------------------------------------------------
unsigned char r_name_table( int adr)
{
	int page_idx;
	
	page_idx = ( adr >> 10) & 3;
	
  /*
  // mirroring = 0 (single page)
  if( page_idx == 0){
    return name_table[ 0 * 0x400 + (adr & 0x3ff)];
  } else if( page_idx == 1){
    return name_table[ 0 * 0x400 + (adr & 0x3ff)];
  } else if( page_idx == 2){
    return name_table[ 0 * 0x400 + (adr & 0x3ff)];
  } else {
    return name_table[ 0 * 0x400 + (adr & 0x3ff)];
  }
  */

  
  // for mirroring = 1 (vertical mirroring)
  if( page_idx == 0){
    return name_table[ 0 * 0x400 + (adr & 0x3ff)];
  } else if( page_idx == 1){
    return name_table[ 1 * 0x400 + (adr & 0x3ff)];
  } else if( page_idx == 2){
    return name_table[ 0 * 0x400 + (adr & 0x3ff)];
  } else {
    return name_table[ 1 * 0x400 + (adr & 0x3ff)];
  }
  

  /*
	// for mirroring = 2 (horizontal mirroring)
	if( page_idx == 0){
		return name_table[ 0 * 0x400 + (adr & 0x3ff)];
	} else if( page_idx == 1){
		return name_table[ 0 * 0x400 + (adr & 0x3ff)];
	} else if( page_idx == 2){
		return name_table[ 1 * 0x400 + (adr & 0x3ff)];
	} else {
		return name_table[ 1 * 0x400 + (adr & 0x3ff)];
	}
  */
}

void w_name_table( int adr, unsigned char dat)
{
	int page_idx;
	
	page_idx = ( adr >> 10) & 3;
  
  /*
  // mirroring = 0 (single page)
  if( page_idx == 0){
    name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
  } else if( page_idx == 1){
    name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
  } else if( page_idx == 2){
    name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
  } else {
    name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
  }    
  */
  	
  // for mirroring = 1 (vertical mirroring)
  if( page_idx == 0){
    name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
  } else if( page_idx == 1){
    name_table[ 1 * 0x400 + (adr & 0x3ff)] = dat;
  } else if( page_idx == 2){
    name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
  } else {
    name_table[ 1 * 0x400 + (adr & 0x3ff)] = dat;
  }  

  /*
	// for mirroring = 2 (horizontal mirroring)
	if( page_idx == 0){
		name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
	} else if( page_idx == 1){
		name_table[ 0 * 0x400 + (adr & 0x3ff)] = dat;
	} else if( page_idx == 2){
		name_table[ 1 * 0x400 + (adr & 0x3ff)] = dat;
	} else {
		name_table[ 1 * 0x400 + (adr & 0x3ff)] = dat;
	}
  */
}

// ------------------------------------------------------------------------------------
//                                     END OF PROGRAM
// ------------------------------------------------------------------------------------
