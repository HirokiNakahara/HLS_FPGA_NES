#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// external variables -----------------------------------------
// for ROM 
int prg_page_cnt;
int chr_page_cnt;
enum { HORIZONTAL, VERTICAL};
int mirroring;
int sram_enable, trainer_enable, four_screen; // bool type
int mapper;
unsigned char *rom_dat, *chr_dat, *sram, *vram;

// #######################################################################
// 
// Partition NES ROM into 
// 
// #######################################################################
int main( int argc, char *argv[])
{
	// ########################################################
	// Definition for Variables
	// ########################################################
	// local variables
	int i, j;
	int cnt;

	int is_support;
	char fname[256];
	
	FILE *f=fopen( argv[1],"rb");
	if( f == NULL){
		fprintf( stderr, "NO FILE\n"); 
		exit(-1);
	}
	
	char sig[4];
	fread( sig, 1, 4, f);
	
	if( strncmp( sig, "NES\x1A", 4) != 0){
		fclose(f);
		fprintf( stderr, "HEADER ERROR\n");
		exit(-1);
	}

	unsigned char t,u;
	fread( &t, 1,1, f);
	prg_page_cnt = t;
	
	fread( &t, 1, 1, f);
	chr_page_cnt = t;
	
	fread( &t, 1, 1, f);
	fread( &u, 1, 1, f);
	
	mirroring      = (t & 1) ? VERTICAL : HORIZONTAL;
	sram_enable    = (t & 2) != 0;
	trainer_enable = (t & 4)!= 0;
	four_screen    = (t & 8) != 0;
	mapper         = (t >> 4) | (u & 0xf0);
	
	char pad[8];
	fread( pad, 1, 8, f);
	
	int rom_size = 0x4000 * prg_page_cnt;
	int chr_size = 0x2000 * chr_page_cnt;

	if( ( rom_dat = (unsigned char *)malloc(sizeof(unsigned char) * (rom_size+1))) == NULL){
		fprintf( stderr, "CAN'T MALLOC *rom_dat\n");
		exit(-1);
	}
	if( chr_size != 0){
		if( ( chr_dat = (unsigned char *)malloc(sizeof(unsigned char) * (chr_size+1))) == NULL){
			fprintf( stderr, "CAN'T MALLOC *chr_dat\n");
			exit(-1);
		}
	}
	if( ( sram = (unsigned char *)malloc(sizeof(unsigned char) * (0x2000+1))) == NULL){
		fprintf( stderr, "CAN'T MALLOC *sram\n");
		exit(-1);
	}
	
	fread( rom_dat, sizeof(unsigned char), rom_size, f);

	if( chr_size != 0){
		fread( chr_dat, sizeof(unsigned char), chr_size, f);
	}

	// ROMのステータスを表示、対応しているかチェックする
	is_support = 1;
	printf("[ROM INFO]\n");
	printf("ROM_SIZE: %d KB, VROM_SIZE: %d KB\n", rom_size / 1024, chr_size / 1024);
	printf("PROGRAM #PAGE: %d, CHARACTER #PAGE: %d\n", prg_page_cnt, chr_page_cnt);
	printf("MAPPER #%d ", mapper);
	if( mapper != 0){
		printf("(NOT SUPPORT...)\n");
		is_support = 0;
	} else {
		printf("\n");
	}
	printf("MIRRORING: %d ", mirroring);
	if( mirroring != 1){
		printf("(NOT SUPPORT...)\n");
		is_support = 0;
	} else {
		printf("\n");
	}
	printf("IS_SRAM: %d ", sram_enable);
	if( sram_enable != 0){
		printf("(NOT SUPPORT...)\n");
		is_support = 0;
	} else {
		printf("\n");
	}
	printf("IS_TRAINER: %d ", trainer_enable);
	if( trainer_enable != 0){
		printf("(NOT SUPPORT...)\n");
		is_support = 0;
	} else {
		printf("\n");
	}
	printf("IS_FOUR_SCREEN: %d ", four_screen);
	if( four_screen != 0){
		printf("(NOT SUPPORT...)\n");
		is_support = 0;
	} else {
		printf("\n");
	}
	fclose( f);
	
	// プログラムROMとキャラクタROMのCコードの配列をそれぞれファイルに出力
	// mbc.c の該当部分(rom_dat,chr_dat)にコピペしてください(対応していれば)
	sprintf( fname, "%s_template.txt", argv[1]);
	if( ( f = fopen( fname, "w")) == NULL){
		fprintf( stderr, "CAN'T OPEN %s\n", fname);
		exit(-1);
	}
	fprintf( f, "unsigned char rom_dat[%d] = {\n", rom_size);
	for( i = 0; i < rom_size; i++){
		if( i != 0)fprintf( f, ", ");
		fprintf( f, "0x%X", rom_dat[i]);
	}
	fprintf( f, "\n};\n");

	fprintf( f, "unsigned char chr_dat[%d] = {\n", chr_size);
	for( i = 0; i < chr_size; i++){
		if( i != 0)fprintf( f, ", ");
		fprintf( f, "0x%X", chr_dat[i]);
	}
	fprintf( f, "\n};");
	fclose(f);
}

//-------------------------------------------------------------------------
//                             END OF PROGRAM
//-------------------------------------------------------------------------
