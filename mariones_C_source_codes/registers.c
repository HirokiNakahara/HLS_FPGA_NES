// -------------------------------------------------------------------------
// registers.c
// 
// Modified by Hiroki Nakahara
// 31, July, 2015
// 
// 各モジュールのレジスタのエミュレーションを行う
// -------------------------------------------------------------------------
#include "mariones.h"
#include <stdio.h>

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

// ########################################################
// Emulation for Registers
// ########################################################
void regs_set_vblank( int b, int nmi)
{
  is_vblank = b;
  if ( nmi && ( !is_vblank || nmi_enable))
    cpu_set_nmi( is_vblank);
  if ( is_vblank)
    sprite0_occur = 0; // boolean
}

void regs_start_frame()
{
  if ( bg_visible || sprite_visible)
    ppu_adr_v = ppu_adr_t;
}

void regs_start_scanline()
{
  if ( bg_visible || sprite_visible)
    ppu_adr_v = ( ppu_adr_v & 0xfbe0) | ( ppu_adr_t & 0x041f);
}

void regs_end_scanline()
{
  if ( bg_visible || sprite_visible){
    if ( (( ppu_adr_v >> 12) & 7) == 7){
      ppu_adr_v &= ~0x7000;
      if ((( ppu_adr_v >> 5) & 0x1f) == 29)
        ppu_adr_v = ( ppu_adr_v & ~0x03e0) ^ 0x800;
      else if ((( ppu_adr_v >> 5) & 0x1f) == 31)
        ppu_adr_v &= ~0x03e0;
      else
        ppu_adr_v += 0x20;
    }
    else
      ppu_adr_v += 0x1000;
  }
}

int regs_draw_enabled() // boolean
{
  return sprite_visible || bg_visible;
}

void regs_set_input( int *dat)
{
	int i;
  	for ( i = 0; i < 16; i++)
    	pad_dat[ i / 8][ i % 8] = dat[ i] ? 1 : 0;
}

// ジョイパッドの読み込みを行う -----------------------------------------------
// １回の読出しでボタンを１個スキャン、ということは１６回読みだされる
// (8ボタン x 2プレイヤ)
// また、PPU（描画ユニット）の制御レジスタも設定する
unsigned char regs_read( int adr)
{
  if ( adr >= 0x4000){
    switch( adr){
    case 0x4016: // Joypad #1 (RW)
    case 0x4017: // Joypad #2 (RW)
      {
        int pad_num = adr - 0x4016;
        int read_pos = joypad_read_pos[ pad_num];
        
        unsigned char ret;
        if ( read_pos < 8) // パッドデータ
          ret = pad_dat[ pad_num][ read_pos] ? 1 : 0;
        else if ( read_pos < 16) // Ignored
          ret = 0;
        else if ( read_pos < 20) // Signature
          ret = ( joypad_sign[ pad_num] >> ( read_pos - 16)) & 1;
        else
          ret = 0;
        joypad_read_pos[ pad_num]++;
        if ( joypad_read_pos[ pad_num] == 24)
          joypad_read_pos[ pad_num] = 0;
        
        // ("pad_num=%d read_pos=%d pad_dat=%d ret=%d\n", pad_num, read_pos, pad_dat[pad_num][read_pos], ret);
        
        return ret;
      }
    case 0x4015:
      return 0x0;
    default:
      return 0x0;
    }
  }

  switch( adr & 7){
  case 0: // PPU Control Register #1 (W)
  case 1: // PPU Control Register #2 (W)
  case 3: // SPR-RAM Address Register (W)
  case 4: // SPR-RAM I/O Register (W)
  case 5: // VRAM Address Register #1 (W2)
  case 6: // VRAM Address Register #2 (W2)
    return 0;

  case 2: { // PPU Status Register (R)
    unsigned char ret = _set( is_vblank, 7) | _set( sprite0_occur, 6) | _set( sprite_over, 5) | _set( vram_write_flag, 4);
    regs_set_vblank( 0, 1); 
    ppu_adr_toggle = 0; //false;
    return ret;
  }
  case 7: { // VRAM I/O Register (RW)
    unsigned char ret = ppu_read_buf;
    ppu_read_buf = regs_read_2007();
    return ret; }
  }

  return 0;
}

// JoypadとPPUレジスタに値を書き込む ---------------------------------------------
void regs_write( int adr, unsigned char dat)
{
	int i;

  if ( adr >= 0x4000){
    switch( adr){
    case 0x4014: // Sprite DMA Register (W)
      {
        for ( i = 0; i < 0x100; i++)
          sprram[ i] = mbc_read( ( dat << 8) | i);
      }
      break;
    case 0x4016: // Joypad #1 (RW)
      {
        int newval = ( dat & 1) != 0;
        if ( joypad_strobe && !newval){ // たち下りエッジでリセット
          joypad_read_pos[ 0] = joypad_read_pos[ 1] =0;
          //printf("check\n");
        }
        joypad_strobe = newval;
      }
      break;
    case 0x4017: // Joypad #2 (RW)
      frame_irq = dat;
      break;
    default:
      break;
    }
    return;
  }

  switch( adr & 7){
  case 0: // PPU Control Register #1 (W)
    nmi_enable    = _bit( dat, 7);
    //ppu_master    = _bit( dat, 6); // unused
    sprite_size   = _bit( dat, 5);
    bg_pat_adr    = _bit( dat, 4);
    sprite_pat_adr= _bit( dat, 3);
    ppu_adr_incr  = _bit( dat, 2);
    //name_tbl_adr  =dat&3;
    ppu_adr_t = ( ppu_adr_t & 0xf3ff) | (( dat & 3) << 10);
    break;
  case 1: // PPU Control Register #2 (W)
    bg_color      = dat >> 5;
    sprite_visible= _bit( dat, 4);
    bg_visible    = _bit( dat, 3);
    sprite_clip   = _bit( dat, 2);
    bg_clip       = _bit( dat, 1);
    color_display = _bit( dat, 0);
    break;
  case 2: // PPU Status Register (R)
    // 未実装…ごめんなさい
    break;
  case 3: // SPR-RAM Address Register (W)
    sprram_adr = dat;
    break;
  case 4: // SPR-RAM I/O Register (W)
    sprram[ sprram_adr++] = dat;
    break;
  case 5: // VRAM Address Register #1 (W2)
    ppu_adr_toggle =! ppu_adr_toggle;
    if ( ppu_adr_toggle){
      ppu_adr_t = ( ppu_adr_t & 0xffe0) | ( dat >> 3);
      ppu_adr_x = dat & 7;
    }
    else{
      ppu_adr_t = ( ppu_adr_t & 0xfC1f) | ( (dat >> 3) << 5);
      ppu_adr_t = ( ppu_adr_t & 0x8fff) | ( (dat & 7) << 12);
    }
    break;
  case 6: // VRAM Address Register #2 (W2)
    ppu_adr_toggle =! ppu_adr_toggle;
    if ( ppu_adr_toggle)
      ppu_adr_t = ( ppu_adr_t & 0x00ff) | (( dat & 0x3f) << 8);
    else{
      ppu_adr_t = ( ppu_adr_t & 0xff00) | dat;
      ppu_adr_v = ppu_adr_t;
    }
    break;
  case 7: // VRAM I/O Register (RW)
    regs_write_2007( dat);
    break;
  }
}

// PPUが各メモリから読み込みを行う ---------------------------------------------------
unsigned char regs_read_2007()
{
  int adr = ppu_adr_v & 0x3fff;
  ppu_adr_v += ppu_adr_incr ? 32 : 1;
  if ( adr < 0x2000)
    return mbc_read_chr_rom( adr); // キャラクタROM
  else if ( adr < 0x3f00)
    return r_name_table(adr); // ネームテーブル
  else{
    if ( (adr & 3) == 0) adr &= ~0x10;
    return palette[ adr & 0x1f]; // パレットテーブル
  }
}

// 逆にこっちはPPUからの書き込み ---------------------------------------------------
void regs_write_2007( unsigned char dat)
{
  int adr = ppu_adr_v & 0x3fff;
  ppu_adr_v += ppu_adr_incr ? 32 : 1;
  
  // printf("adr=%x ppu_adr_v=%x\n", adr, ppu_adr_v);
  
  if ( adr < 0x2000)
    mbc_write_chr_rom( adr, dat);
  else if ( adr < 0x3f00) // name table
    w_name_table( adr, dat);
  else{
    if (( adr & 3) == 0) adr &= ~0x10; // mirroringの処理
    palette[ adr & 0x1f] = dat & 0x3f;
  }
}
// ------------------------------------------------------------------------------------
//                                     END OF PROGRAM
// ------------------------------------------------------------------------------------
