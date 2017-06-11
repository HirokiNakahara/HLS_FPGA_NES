// -------------------------------------------------------------------------
// mbc.c
// 
// Developed by Hiroki Nakahara
// 31, July, 2015
// 
// ROMカートリッジをエミュレーション. 別プログラム partition_ROM.exe を利用して
// ROMをrom_dat, chr_datに分割してここにコピペしてください
// -------------------------------------------------------------------------
#include "mariones.h"
#include <stdio.h>
#include <stdlib.h>

/*
// カートリッジイメージを別配布の partition_ROM.exe を利用して生成したテンプレートから
// ここにコピペする. スーパーマリオしか確認していません…。
unsigned char rom_dat[32768] = {
};
unsigned char chr_dat[8192] = {
};
*/

// ########################################################
// Emulation for MBC
// ########################################################
unsigned char mbc_read( int adr)
{
  switch( adr >> 11){
  case 0x00: case 0x01: case 0x02: case 0x03: //0x0000～0x1FFF
    if( (adr & 0x7FF) == 0x742){
  		// printf("read RAM[0x742] = %d\n", ram[ adr & 0x7ff]);
  	}
    return ram[ adr & 0x7ff];
  case 0x04: case 0x05: case 0x06: case 0x07: // 0x2000～0x3FFF
    return regs_read( adr);
  case 0x08: case 0x09: case 0x0A: case 0x0B: // 0x4000～0x5FFF
    if ( adr < 0x4020) return regs_read( adr);
    return 0;
  case 0x0C: case 0x0D: case 0x0E: case 0x0F: // 0x6000～0x7FFF
  /* this version does not support SRAM
    if ( sram_enabled)
      return sram[adr&0x1fff];
    else
  */
    return 0x00; // ???

  case 0x10: case 0x11: case 0x12: case 0x13: // 0x8000～0x9FFF
    return rom_dat[ 0 * 0x2000 + ( adr & 0x1fff)];
  case 0x14: case 0x15: case 0x16: case 0x17: // 0xA000～0xBFFF
    return rom_dat[ 1 * 0x2000 + ( adr & 0x1fff)];
  case 0x18: case 0x19: case 0x1A: case 0x1B: // 0xC000～0xDFFF
    return rom_dat[ 2 * 0x2000 + ( adr & 0x1fff)];
  case 0x1C: case 0x1D: case 0x1E: case 0x1F: // 0xE000～0xFFFF
    return rom_dat[ 3 * 0x2000 + ( adr & 0x1fff)];
  }
}

void mbc_write( int adr, unsigned char dat)
{
  switch( adr >> 11){
  case 0x00: case 0x01: case 0x02: case 0x03: // 0x0000～0x1FFF
  	/*
  	if( (adr & 0x7FF) == 0x742){
  		printf("write RAM[0x742] = %d\n", dat);
  		
  		if( dat == 2)
  			exit(0);
  		//exit(0);
  	}
  	*/
    ram[ adr & 0x7ff] = dat;
    break;
  case 0x04: case 0x05: case 0x06: case 0x07: // 0x2000～0x3FFF
    regs_write( adr, dat);
    break;
  case 0x08: case 0x09: case 0x0A: case 0x0B: // 0x4000～0x5FFF
    if ( adr < 0x4020) regs_write( adr, dat);
    break;
  case 0x0C: case 0x0D: case 0x0E: case 0x0F: // 0x6000～0x7FFF
    break;

  case 0x10: case 0x11: case 0x12: case 0x13: // 0x8000～0xFFFF
  case 0x14: case 0x15: case 0x16: case 0x17:
  case 0x18: case 0x19: case 0x1A: case 0x1B:
  case 0x1C: case 0x1D: case 0x1E: case 0x1F:
    break;
  }
}

unsigned char mbc_read_chr_rom( int adr)
{
  return chr_dat[ (( adr >> 10) & 7) * 0x400 + ( adr & 0x03ff)];
}

void mbc_write_chr_rom( int adr, int dat)
{
  chr_dat[ (( adr >> 10) & 7) * 0x400 + ( adr & 0x03ff)] = dat;
}
// --------------------------------------------------------------------------
//                             END OF PROGRAM
// --------------------------------------------------------------------------
