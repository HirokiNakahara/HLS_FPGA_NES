// -------------------------------------------------------------------------
// mariones.h
// 
// Modified by Hiroki Nakahara
// 31, July, 2015
// 
// 前半：　CPUの命令のマクロを定義
// 後半：　各モジュールのレジスタ・メモリを外部変数として定義
//
// 注意: この実装ではマッパー番号, ミラーリングはあらかじめ得られたものとしています
//       マリオだったら mapper=0, mirroring=1 など
//       カートリッジのイメージ毎に 別プログラム partition_ROM.exe で読み込んで判別してください
//       一応, マッパ０なら動くはずなんですが…未確認。
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// CPU命令セットのマクロ定義
// -------------------------------------------------------------------------

// define utility functions
#define _bit(x,n) (((x>>n)&1)!=0)
#define _set(b,n) ((b?1:0)<<n)

// define cpu instructions
#define _imm() (reg_pc++)

// TODO 桁上がりのペナルティー
#define _abs()  (reg_pc+=2,cpu_read16(opr_pc))
#define _abxi() (reg_pc+=2,cpu_read16(cpu_read16(opr_pc)+reg_x))
#define _abx()  (reg_pc+=2,cpu_read16(opr_pc)+reg_x)
#define _aby()  (reg_pc+=2,cpu_read16(opr_pc)+reg_y)
#define _absi() (reg_pc+=2,cpu_read16(cpu_read16(opr_pc)))

#define _zp()   (cpu_read8(reg_pc++))
#define _zpxi() (cpu_read16((unsigned char)(cpu_read8(reg_pc++)+reg_x)))
#define _zpx()  ((unsigned char)(cpu_read8(reg_pc++)+reg_x))
#define _zpy()  ((unsigned char)(cpu_read8(reg_pc++)+reg_y))
#define _zpi()  (cpu_read16(cpu_read8(reg_pc++)))
#define _zpiy() (cpu_read16(cpu_read8(reg_pc++))+reg_y)

#define _push8(dat)  cpu_write8(0x100|(unsigned char)(reg_s--),dat)
#define _pop8()      cpu_read8(0x100|(unsigned char)(++reg_s))
#define _push16(dat) (cpu_write16(0x100|(unsigned char)(reg_s-1),dat),reg_s-=2)
#define _pop16()     (reg_s+=2,cpu_read16(0x100|(unsigned char)(reg_s-1)))

#define _bind_flags() ((n_flag<<7)|(v_flag<<6)|0x20|(b_flag<<4)|(d_flag<<3)|(i_flag<<2)|(z_flag<<1)|c_flag)
#define _unbind_flags(dd) { \
  unsigned char dat=dd; \
  n_flag=dat>>7; \
  v_flag=(dat>>6)&1; \
  b_flag=(dat>>4)&1; \
  d_flag=(dat>>3)&1; \
  i_flag=(dat>>2)&1; /* (iがクリアされた場合、割り込まれる可能性が) */ \
  z_flag=(dat>>1)&1; \
  c_flag=dat&1; \
}

// TODO : decimal support
#define _adc(cycle,adr) { \
  unsigned char  s=cpu_read8(adr); \
  unsigned int t=reg_a+s+c_flag; \
  c_flag=(unsigned char)(t>>8); \
  z_flag=(t&0xff)==0; \
  n_flag=(t>>7)&1; \
  v_flag=!((reg_a^s)&0x80)&&((reg_a^t)&0x80); \
  reg_a=(unsigned char)t; \
  rest-=cycle; \
}
// TODO : decimal support
#define _sbc(cycle,adr) { \
  unsigned char s=cpu_read8(adr); \
  unsigned int t=reg_a-s-(c_flag?0:1); \
  c_flag=t<0x100; \
  z_flag=(t&0xff)==0; \
  n_flag=(t>>7)&1; \
  v_flag=((reg_a^s)&0x80)&&((reg_a^t)&0x80); \
  reg_a=(unsigned char)t; \
  rest-=cycle; \
}
#define _cmp(cycle,reg,adr) { \
  unsigned int t=reg-cpu_read8(adr); \
  c_flag=t<0x100; \
  z_flag=(t&0xff)==0; \
  n_flag=(t>>7)&1; \
  rest-=cycle; \
}

#define _and(cycle,adr) { \
  reg_a&=cpu_read8(adr); \
  n_flag=reg_a>>7; \
  z_flag=reg_a==0; \
  rest-=cycle; \
}
#define _ora(cycle,adr) { \
  reg_a|=cpu_read8(adr); \
  n_flag=reg_a>>7; \
  z_flag=reg_a==0; \
  rest-=cycle; \
}
#define _eor(cycle,adr) { \
  reg_a^=cpu_read8(adr); \
  n_flag=reg_a>>7; \
  z_flag=reg_a==0; \
  rest-=cycle; \
}

#define _cpubit(cycle,adr) { \
  unsigned char t=cpu_read8(adr); \
  n_flag=t>>7; \
  v_flag=(t>>6)&1; \
  z_flag=(reg_a&t)==0; \
  rest-=cycle; \
}


#define _load(cycle,reg,adr) { \
  reg=cpu_read8(adr); \
  n_flag=reg>>7; \
  z_flag=reg==0; \
  rest-=cycle; \
}
#define _store(cycle,reg,adr) { \
  cpu_write8(adr,reg); \
  rest-=cycle; \
}

#define _mov(cycle,dest,src) { \
  dest=src; \
  n_flag=src>>7; \
  z_flag=src==0; \
  rest-=cycle; \
}

#define _asli(arg) \
  c_flag=arg>>7; \
  arg<<=1; \
  n_flag=arg>>7; \
  z_flag=arg==0;
#define _lsri(arg) \
  c_flag=arg&1; \
  arg>>=1; \
  n_flag=arg>>7; \
  z_flag=arg==0;
#define _roli(arg) \
  unsigned char u=arg; \
  arg=(arg<<1)|c_flag; \
  c_flag=u>>7; \
  n_flag=arg>>7; \
  z_flag=arg==0;
#define _rori(arg) \
  unsigned char u=arg; \
  arg=(arg>>1)|(c_flag<<7); \
  c_flag=u&1; \
  n_flag=arg>>7; \
  z_flag=arg==0;
#define _inci(arg) \
  arg++; \
  n_flag=arg>>7; \
  z_flag=arg==0;
#define _deci(arg) \
  arg--; \
  n_flag=arg>>7; \
  z_flag=arg==0;

#define _sfta(cycle,reg,op) { op(reg);rest-=cycle; }
#define _sft(cycle,adr,op) { \
  unsigned int a=adr; \
  unsigned char t=cpu_read8(a); \
  op(t); \
  cpu_write8(a,t); \
  rest-=cycle; \
}

#define _asla(cycle)    _sfta(cycle,reg_a,_asli)
#define _asl(cycle,adr) _sft(cycle,adr,_asli)
#define _lsra(cycle)    _sfta(cycle,reg_a,_lsri)
#define _lsr(cycle,adr) _sft(cycle,adr,_lsri)
#define _rola(cycle)    _sfta(cycle,reg_a,_roli)
#define _rol(cycle,adr) _sft(cycle,adr,_roli)
#define _rora(cycle)    _sfta(cycle,reg_a,_rori)
#define _ror(cycle,adr) _sft(cycle,adr,_rori)

#define _incr(cycle,reg) _sfta(cycle,reg,_inci)
#define _inc(cycle,adr)  _sft(cycle,adr,_inci)
#define _decr(cycle,reg) _sfta(cycle,reg,_deci)
#define _dec(cycle,adr)  _sft(cycle,adr,_deci)

#define _bra(cycle,cond) { \
  signed char rel=(signed char)cpu_read8(_imm()); \
  rest-=cycle; \
  if (cond){ \
    rest-=(reg_pc&0xff00)==((reg_pc+rel)&0xff)?1:2; \
    reg_pc+=rel; \
  } \
}

// ----------------------------------------------------------------------
// 定数、主にBMP画像出力用に使う
// ----------------------------------------------------------------------
#define PIXEL_NUM_X (256)	/* 画像のXサイズ */
#define PIXEL_NUM_Y (240)	/* 画像のYサイズ */
#define COLOR_BIT (24)	/* 色ビット */
#define PIC_DATA_SIZE (PIXEL_NUM_X * 3 * PIXEL_NUM_Y)	/* bitmapのサイズ */

// ----------------------------------------------------------------------
// 外部変数の定義
// レジスタやメモリ用
// ----------------------------------------------------------------------
// for debug
int frame_number;
    
// ROM ------------------------------------------------------------------
int prg_page_cnt;
int chr_page_cnt;
//int mirroring; // 今回の実装では定数として指定。本来は各カートリッジに対応すべきなんですが…
int sram_enable, trainer_enable, four_screen; // bool type
int mapper;

unsigned char *sram;
unsigned char *vram;

// MBC ------------------------------------------------------------------
unsigned char ram[ 0x800 + 1];
int is_vram;
	
// Registers ------------------------------------------------------------
int nmi_enable; // boolean
int sprite_size; // boolean
int bg_pat_adr, sprite_pat_adr; // boolean
int ppu_adr_incr; // boolean
int name_tbl_adr;

int bg_color;
int sprite_visible, bg_visible; // boolean
int sprite_clip, bg_clip; // boolean
int color_display; // boolean

int is_vblank; // boolean
int sprite0_occur, sprite_over; // boolean
int vram_write_flag; // boolean

unsigned char sprram_adr;
unsigned int ppu_adr_t, ppu_adr_v, ppu_adr_x;
int ppu_adr_toggle;  // boolean
unsigned char ppu_read_buf;

int joypad_strobe;  // boolean
int joypad_read_pos[2], joypad_sign[2];
int frame_irq;
int pad_dat[2][8]; // boolean
	
// CPU -------------------------------------------------------------------
unsigned char reg_a, reg_x,reg_y, reg_s;
unsigned int reg_pc;
unsigned char c_flag, z_flag, i_flag, d_flag, b_flag, v_flag, n_flag;

int rest;
unsigned int mclock; // 64 bit??
int nmi_line, irq_line, reset_line; // boolean

unsigned char opc;

int exec_MNI;

// PPU -------------------------------------------------------------------
unsigned char sprram[0x100];
unsigned char name_table[0x1000];
unsigned char palette[0x20];

int nes_palette_24[0x40];
int nes_palette_16[0x40];

// ------------------------------------------------------------------------------------
//                               END OF HEADER FILE
// ------------------------------------------------------------------------------------
