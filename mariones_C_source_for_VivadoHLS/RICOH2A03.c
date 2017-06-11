// -------------------------------------------------------------------------
// RICHO2A03.c
// 
// Modified by Hiroki Nakahara
// 31, July, 2015
// 
// NESで使ってるCPU(6502)コンパチのプロセッサをエミュレーションする
// といいながら、命令セットはほとんどマクロをつかっているので mariones.hの前半部を見てください
// マリオでしかデバッグしてないので、他のカートリッジで動くかは心配…。
// -------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "mariones.h"

enum irq_type{ NMI, IRQ, RESET};

void cpu_set_nmi( int b);
void cpu_set_irq( int b);
void cpu_set_reset( int b);
unsigned char cpu_read8( int adr);
int cpu_read16( int adr);
void cpu_write8( int adr, unsigned char dat);
void cpu_write16( int adr, int dat);
void cpu_exec( int clk);
void cpu_exec_irq( int it);

// デバッグ用関数, Vivado HLSは標準出力(printf,fprintfとか)は無視してくれるのでタスカッタ…
int cpu_get_master_clock();
double cpu_get_frequency();
void cpu_log();

// ########################################################
// Emulation for CPU
// ########################################################

// MNI割り込みフラグセット
void cpu_set_nmi( int b) // boolean
{
	/*
  // エッジセンシティブな割り込み
  // 本来はここに記述したんだけど、Vivado HLSが再帰構造になっているぞ！
  // と怒りましたので、フラグだけ立てておいて、メインルーチンで優先的に実行するように
  // 書き換えています..
  if ( !nmi_line && b)
    cpu_exec_irq( NMI);
  nmi_line = b;
  */
  exec_MNI = 1;
}

// IRQ割り込みセット
// おそらくレベルセンシティブな割り込み
// 処理の簡便のために、命令フェッチ前に割り込みチェックをする
void cpu_set_irq( int b) // boolean
{
  irq_line = b;
}

// リセット割り込み
void cpu_set_reset( int b) // boolean
{
  reset_line = b;
}

// MBC読出し
unsigned char cpu_read8( int adr)
{
  return mbc_read( adr);
}

// 16bit読出し
int cpu_read16( int adr)
{
  return cpu_read8( adr) | ( cpu_read8( adr+1) << 8);
}

// 8bit書き込み
void cpu_write8( int adr, unsigned char dat)
{
  mbc_write( adr, dat);
}

// 16bit書き込み
void cpu_write16( int adr, int dat)
{
  cpu_write8( adr, (unsigned char)dat);
  cpu_write8( adr+1, (unsigned char)(dat >> 8));
}

// -----------------------------------------------------------------------
// クロック実行 (最低一命令は実行する)
// -----------------------------------------------------------------------
void cpu_exec( int clk)
{
  // クロック数残り更新
  rest += clk;
  mclock += clk;

  do{
    // IRQ チェック
    if ( !i_flag){ 
      // 割り込みがかかるとラインのレベルを落とす。本当は違うんだろうけど。
      if ( reset_line) cpu_exec_irq( RESET), reset_line = 0;
      else if ( irq_line) cpu_exec_irq( IRQ), irq_line = 0;//false;
    }
    
    // 命令読出し
    opc = cpu_read8( reg_pc++);
    int opr_pc = reg_pc;
    
    //printf("opc=%X\n", opc);
    
    //if( frame_number == 30){
	   /*	
	   printf(" ** OP:%X %X A:%X X:%X Y:%X S:%X n=%x v=%x b=%x d=%x i=%x z=%x c=%x\n",
				opc, reg_pc, reg_a, reg_x, reg_y, reg_s, n_flag, v_flag, b_flag, d_flag, i_flag, z_flag, c_flag);
	   */			
	 //}
	 
   // CPUのデバッグしたいときは↓を呼び出してね
	 //cpu_log();
	
  // --------------------------------------------------------------------------
  // 命令コードに応じて命令実行する
  // --------------------------------------------------------------------------
    switch( opc){
      /* ALU */
    case 0x69: _adc( 2, _imm());  break;
    case 0x65: _adc( 3, _zp());   break;
    case 0x75: _adc( 4, _zpx());  break;
    case 0x6D: _adc( 4, _abs());  break;
    case 0x7D: _adc( 4, _abx());  break;
    case 0x79: _adc( 4, _aby());  break;
    case 0x61: _adc( 6, _zpxi()); break;
    case 0x71: _adc( 5, _zpiy()); break;

    case 0xE9: _sbc( 2, _imm());  break;
    case 0xE5: _sbc( 3, _zp());   break;
    case 0xF5: _sbc( 4, _zpx());  break;
    case 0xED: _sbc( 4, _abs());  break;
    case 0xFD: _sbc( 4, _abx());  break;
    case 0xF9: _sbc( 4, _aby());  break;
    case 0xE1: _sbc( 6, _zpxi()); break;
    case 0xF1: _sbc( 5, _zpiy()); break;

    case 0xC9: _cmp( 2, reg_a, _imm());  break;
    case 0xC5: _cmp( 3, reg_a, _zp());   break;
    case 0xD5: _cmp( 4, reg_a, _zpx());  break;
    case 0xCD: _cmp( 4, reg_a, _abs());  break;
    case 0xDD: _cmp( 4, reg_a, _abx());  break;
    case 0xD9: _cmp( 4, reg_a, _aby());  break;
    case 0xC1: _cmp( 6, reg_a, _zpxi()); break;
    case 0xD1: _cmp( 5, reg_a, _zpiy()); break;

    case 0xE0: _cmp( 2, reg_x, _imm()); break;
    case 0xE4: _cmp( 2, reg_x, _zp());  break;
    case 0xEC: _cmp( 3, reg_x, _abs()); break;

    case 0xC0: _cmp( 2, reg_y, _imm()); break;
    case 0xC4: _cmp( 2, reg_y, _zp());  break;
    case 0xCC: _cmp( 3, reg_y, _abs()); break;

    case 0x29: _and( 2, _imm());  break;
    case 0x25: _and( 3, _zp());   break;
    case 0x35: _and( 4, _zpx());  break;
    case 0x2D: _and( 4, _abs());  break;
    case 0x3D: _and( 4, _abx());  break;
    case 0x39: _and( 4, _aby());  break;
    case 0x21: _and( 6, _zpxi()); break;
    case 0x31: _and( 5, _zpiy()); break;

    case 0x09: _ora( 2, _imm());  break;
    case 0x05: _ora( 3, _zp());   break;
    case 0x15: _ora( 4, _zpx());  break;
    case 0x0D: _ora( 4, _abs());  break;
    case 0x1D: _ora( 4, _abx());  break;
    case 0x19: _ora( 4, _aby());  break;
    case 0x01: _ora( 6, _zpxi()); break;
    case 0x11: _ora( 5, _zpiy()); break;

    case 0x49: _eor( 2, _imm());  break;
    case 0x45: _eor( 3, _zp());   break;
    case 0x55: _eor( 4, _zpx());  break;
    case 0x4D: _eor( 4, _abs());  break;
    case 0x5D: _eor( 4, _abx());  break;
    case 0x59: _eor( 4, _aby());  break;
    case 0x41: _eor( 6, _zpxi()); break;
    case 0x51: _eor( 5, _zpiy()); break;

    case 0x24: _cpubit( 3, _zp());   break;
    case 0x2C: _cpubit( 4, _abs());  break;

      /* laod / store */
    case 0xA9: _load( 2, reg_a, _imm());  break;
    case 0xA5: _load( 3, reg_a, _zp());   break;
    case 0xB5: _load( 4, reg_a, _zpx());  break;
    case 0xAD: _load( 4, reg_a, _abs());  break;
    case 0xBD: _load( 4, reg_a, _abx());  break;
    case 0xB9: _load( 4, reg_a, _aby());  break;
    case 0xA1: _load( 6, reg_a, _zpxi()); break;
    case 0xB1: 
    	// printf("reg_pc=%x read8=%x read16=%x reg_a=%x\n",reg_pc,cpu_read8(reg_pc),cpu_read16(cpu_read8(reg_pc)), reg_a); 
    _load( 5, reg_a, _zpiy()); 
    	// printf(" after reg_a=%x read8(0x403)=%x\n", reg_a, cpu_read8( 0x403));
    break;

    case 0xA2: _load( 2, reg_x, _imm());  break;
    case 0xA6: _load( 3, reg_x, _zp());   break;
    case 0xB6: _load( 4, reg_x, _zpy());  break;
    case 0xAE: _load( 4, reg_x, _abs());  break;
    case 0xBE: _load( 4, reg_x, _aby());  break;

    case 0xA0: _load( 2, reg_y, _imm());  break;
    case 0xA4: _load( 3, reg_y, _zp());   break;
    case 0xB4: _load( 4, reg_y, _zpx());  break;
    case 0xAC: _load( 4, reg_y, _abs());  break; //printf("opr_pc=%x read16=%x\n", opr_pc, cpu_read16( opr_pc)); break;
    case 0xBC: _load( 4, reg_y, _abx());  break;

    case 0x85: _store( 3, reg_a, _zp());   break;
    case 0x95: _store( 4, reg_a, _zpx());  break;
    case 0x8D: _store( 4, reg_a, _abs());  break;
    case 0x9D: _store( 5, reg_a, _abx());  break;
    case 0x99: _store( 5, reg_a, _aby());  break;
    case 0x81: _store( 6, reg_a, _zpxi()); break;
    case 0x91: _store( 6, reg_a, _zpiy()); break;

    case 0x86: _store( 3, reg_x, _zp());   break;
    case 0x96: _store( 4, reg_x, _zpy());  break;
    case 0x8E: _store( 4, reg_x, _abs());  break;

    case 0x84: _store( 3, reg_y, _zp());   break;
    case 0x94: _store( 4, reg_y, _zpx());  break;
    case 0x8C: _store( 4, reg_y, _abs());  break;

      /* transfer */
    case 0xAA: _mov( 2, reg_x, reg_a); break; // TAX
    case 0xA8: _mov( 2, reg_y, reg_a); break; // TAY
    case 0x8A: _mov( 2, reg_a, reg_x); break; // TXA
    case 0x98: _mov( 2, reg_a, reg_y); break; // TYA
    case 0xBA: _mov( 2, reg_x, reg_s); break; // TSX
    case 0x9A: reg_s = reg_x; rest -= 2; break; // TXS

      /* shift */
    case 0x0A: _asla( 2);       break;
    case 0x06: _asl( 5, _zp());  break;
    case 0x16: _asl( 6, _zpx()); break;
    case 0x0E: _asl( 6, _abs()); break;
    case 0x1E: _asl( 7, _abx()); break;

    case 0x4A: _lsra( 2);       break;
    case 0x46: _lsr( 5, _zp());  break;
    case 0x56: _lsr( 6, _zpx()); break;
    case 0x4E: _lsr( 6, _abs()); break;
    case 0x5E: _lsr( 7, _abx()); break;

    case 0x2A: _rola( 2);       break;
    case 0x26: _rol( 5, _zp());  break;
    case 0x36: _rol( 6, _zpx()); break;
    case 0x2E: _rol( 6, _abs()); break;
    case 0x3E: _rol( 7, _abx()); break;

    case 0x6A: _rora( 2);       break;
    case 0x66: _ror( 5, _zp());  break;
    case 0x76: _ror( 6, _zpx()); break;
    case 0x6E: _ror( 6, _abs()); break;
    case 0x7E: _ror( 7, _abx()); break;

    case 0xE6: _inc( 5, _zp());  break;
    case 0xF6: _inc( 6, _zpx()); break;
    case 0xEE: _inc( 6, _abs()); break;
    case 0xFE: _inc( 7, _abx()); break;
    case 0xE8: _incr( 2, reg_x); break;
    case 0xC8: _incr( 2, reg_y); break;

    case 0xC6: _dec( 5, _zp());  break;
    case 0xD6: _dec( 6, _zpx()); break;
    case 0xCE: _dec( 6, _abs()); break;
    case 0xDE: _dec( 7, _abx()); break;
    case 0xCA: _decr( 2, reg_x); break;
    case 0x88: _decr( 2, reg_y); break;

      /* branch */
    case 0x90: _bra( 2, !c_flag); break; // BCC
    case 0xB0: _bra( 2,  c_flag); break; // BCS
    case 0xD0: _bra( 2, !z_flag); break; // BNE
    case 0xF0: _bra( 2,  z_flag); break; // BEQ
    case 0x10: _bra( 2, !n_flag); break; // BPL
    case 0x30: _bra( 2,  n_flag); break; // BMI
    case 0x50: _bra( 2, !v_flag); break; // BVC
    case 0x70: _bra( 2,  v_flag); break; // BVS

      /* jump / call / return */
    case 0x4C: reg_pc = _abs() ; rest -= 3; break; // JMP abs
    case 0x6C: reg_pc = _absi(); rest -= 5; break; // JMP (abs)

    case 0x20: _push16( reg_pc + 1); reg_pc = _abs(); rest -= 6; break; // JSR

    case 0x60: reg_pc = _pop16() + 1; rest -= 6; break; // RTS
    case 0x40: _unbind_flags( _pop8()); reg_pc = _pop16(); rest -= 6; break; // RTI

      /* flag */
    case 0x38: c_flag = 1; rest -= 2; break; // SEC
    case 0xF8: d_flag = 1; rest -= 2; break; // SED
    case 0x78: i_flag = 1; rest -= 2; break; // SEI

    case 0x18: c_flag = 0; rest -= 2; break; // CLC
    case 0xD8: d_flag = 0; rest -= 2; break; // CLD
    case 0x58: i_flag = 0; rest -= 2; break; // CLI (この瞬間に割り込みがかかるかも知れん…)
    case 0xB8: v_flag = 0; rest -= 2; break; // CLV

      /* stack */
    case 0x48: _push8( reg_a); rest -= 3; break; // PHA
    case 0x08: _push8( _bind_flags()); rest -= 3; break; // PHP
    case 0x68: reg_a = _pop8(); n_flag = reg_a >> 7; z_flag = reg_a == 0; rest -= 4; break; // PLA
    case 0x28: _unbind_flags( _pop8()); rest -= 4; break; // PLP

      /* others */
    case 0x00: // BRK
      b_flag = 1;
      reg_pc++;
      cpu_exec_irq( IRQ);
      break;

    case 0xEA: rest -= 2; break; // NOP

    default:
      // fprintf( stderr, "Undefined Operation is called!!\n");
      break;
    }
  }while( rest > 0);
  // CPU命令コードエミュレーション終了 -------------------------------------------------
}

// -----------------------------------------------------------------------------
// 割り込み実行
// -----------------------------------------------------------------------------
void cpu_exec_irq( int it)//IRQ_TYPE it)
{
//  if (logging)
//    cout<<(it==RESET?"RESET":it==NMI?"NMI":"IRQ")<<" occured !"<<endl;

  unsigned int vect = (
    (it == RESET) ? 0xFFFC :
    (it == NMI  ) ? 0xFFFA :
    (it == IRQ  ) ? 0xFFFE : 0xFFFE);
  _push16( reg_pc);
  _push8( _bind_flags());
  i_flag = 1;
  reg_pc = cpu_read16( vect);
  
  //printf("reg_pc=%x\n", reg_pc);
  
  rest -= 7;
}

// -----------------------------------------------------------------------------
// デバッグ用関数, Vivado HLSは標準出力(printf,fprintfとか)は無視してくれるのでタスカッタ…
// -----------------------------------------------------------------------------

int cpu_get_master_clock()
{
  return mclock - rest;
}

double cpu_get_frequency()
{
  return 3579545.0/2;
}

void cpu_log()
{
	//printf("OP:%X %X A:%X X:%X Y:%X S:%X P:", opc, reg_pc, reg_a, reg_x, reg_y, reg_s);
	printf("OP:%X %X ", opc, reg_pc);
	
	if( reg_a >= 16){
		printf("A:%X ", reg_a);
	} else {
		printf("A:0%X ", reg_a);
	}
	if( reg_x >= 16){
		printf("X:%X ", reg_x);
	} else {
		printf("X:0%X ", reg_x);
	}
	if( reg_y >= 16){
		printf("Y:%X ", reg_y);
	} else {
		printf("Y:0%X ", reg_y);
	}
	if( reg_s >= 16){
		printf("S:%X ", reg_s);
	} else {
		printf("S:0%X ", reg_s);
	}
	
	printf("P:");
	( n_flag == 1) ? printf("N") : printf("n");
	( v_flag == 1) ? printf("V") : printf("v");
	( b_flag == 1) ? printf("B") : printf("b");
	( d_flag == 1) ? printf("D") : printf("d");
	( i_flag == 1) ? printf("I") : printf("i");
	( z_flag == 1) ? printf("Z") : printf("z");
	( c_flag == 1) ? printf("C") : printf("c");
	printf("\n");
}
// ------------------------------------------------------------------------------------
//                                     END OF PROGRAM
// ------------------------------------------------------------------------------------
