// -----------------------------------------------------------------------------
// main.c
// 
// Developed by Hiroki Nakahara
// 31, July, 2015
// 
// SuperMairoBros. 専用NESエミュレータのテストベンチ
// NES自身は mariones_top.c なので, Vivado HLSでの合成対象も mariones_top.c
// main.cはテストベンチとしてVivado HLSに読み込んでおくこと
// mov_mario.txtにジョイパッドシーケンスを入れているので、これもVivado HLSテストベンチで読込む
// 
// BMP画像を10フレーム毎に出力することでデバッグする。
// RICOH2A03.c のコメントを外すと CPUのレジスタ等を観測することが可能。
// 何フレームシミュレーションするかは, 外部変数 (max_frame)を直接変えてください
//
// 注意： NES自身は35～40フレーム実行しないと画面表示しないので待つこと。
//  ROMイメージは各自用意してね。私はArduinoを使って作りました。ネットからダウンロードしたら違法だよ！
//  ROMイメージは私が用意した partition_ROM.exe でプログラムROMとキャラクタROMに分割してから
//  mbc.c の先頭の配列を書き換えてください。
// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NES共通のヘッダ
#include "mariones.h"

/* 任意精度型を使うために必要なヘッダ！ */
#include "ap_cint.h"

// エミュレーションするフレーム数
int max_frame = 400;

/* NES本体                           */
/* メモリをケチるために任意精度型でキツク制限 */
int mariones_top( 
	uint1 reset, // ソフトウェアリセット
	uint12 bmp[256*240], // 画面描画。これをBMPに変換する
	unsigned char pad0,  //ジョイパッド1の状態, 8ビット
	unsigned char pad1  //ジョイパッド2の状態, 8ビット 
);


// BMP関連の関数
int putBmpHeader(FILE *s, int x, int y, int c);
int fputc2LowHigh(unsigned short d, FILE *s);
int fputc4LowHigh(unsigned long d, FILE *s);

// #######################################################################
// 
// TestBench(Top Function) for NES emulator
// 
// #######################################################################
int main()
{
	// ########################################################
	// Definition for Variables
	// ########################################################
	// local variables
	int i, j;
	int x, y;
	int cnt;
	int reset;
	
	int temp_frame_num;

	FILE *fp_bmp;
	FILE *fp_mov;
	char line[50];
	unsigned int d1, d2;
	unsigned char *mov;
	
	/* BMP画像とワーク用バッファ */
	uint12 bmp[256*240];
	unsigned char conv_bmp[256*3*240]; // bmpを12ビットにしたので, 32ビット用に新たに用意

	// ジョイパッド入力をテストベンチとして読込み
	// あらかじめ別のエミュで fmv 形式として保存したものをテキストに直して作りました
	// 4000フレームまで
	if( ( mov = (unsigned char *)malloc(sizeof(unsigned char) * 4001)) == NULL){
		fprintf( stderr, "CAN'T MALLOC MOV\n");
		exit(-1);
	}
	memset( mov, 0, 4000);
	
	if( ( fp_mov = fopen("mov_mario.txt", "rt")) == NULL){
		fprintf( stderr, "MOVEMENT READ ERROR\n");
		exit(0);
	}
	
	while( fgets( line, 50, fp_mov) != NULL){
		sscanf( line, "frame %d mov %x\n", &d1, &d2);
		mov[d1] = d2;
	}
	
	fclose( fp_mov);

	// ########################################################
	// Execute NES Emulator
	// ########################################################
	int pad[2][8] = {{0,0,0,0, 0,0,0,0}, {0,0,0,0, 0,0,0,0}};
	
	printf("MAX #FRAME: %d\n", max_frame);
	
	// nes emulation ------------------------------------------
	for( cnt = 0; cnt < max_frame; cnt++){
		//printf("------- frame=%d ------\n", cnt);
		
		// 0フレームはソフトウェアリセットをかける
		if( cnt == 0){
			reset = 1;
		} else {
			reset = 0;
		}

		// NES本体の読出し
		temp_frame_num = mariones_top( reset, bmp, mov[cnt], 0);

		printf("frame=%d temp=%d\n", cnt, temp_frame_num);
		
		// 40フレームを超えたら10フレーム毎にBMP画像として出力
		if( cnt != 0 && cnt % 10 == 0 && cnt >= 40){
			// output BMP file
			char fname[256];
			sprintf(fname, "screen_%d.bmp", cnt);
			fp_bmp = fopen( fname, "wb");
			putBmpHeader( fp_bmp, PIXEL_NUM_X, PIXEL_NUM_Y, COLOR_BIT);

			// convert BMP format
			for( y = 0; y < 240; y++){
				for( x = 0; x < 256; x++){
					conv_bmp[(239 - y) * 256 * 3 + x * 3]
					         = ((bmp[256 * y + x] >> 8) & 0xF) << 4;
					conv_bmp[(239 - y) * 256 * 3 + x * 3 + 1]
					         = ((bmp[256 * y + x] >> 4) & 0xF) << 4;
					conv_bmp[(239 - y) * 256 * 3 + x * 3 + 2]
					         =  (bmp[256 * y + x] & 0xF) << 4;
				}
			}

			fwrite( conv_bmp, sizeof(unsigned char), PIC_DATA_SIZE, fp_bmp);
	
			fclose( fp_bmp);
		}
	}
	// enf of emulation ---------------------------------------
		
	return 0;
}

// ###################################################
// BMP generation
// ###################################################
int putBmpHeader(FILE *s, int x, int y, int c)
{
	int i;
	int color; /* 色数 */
	unsigned long int bfOffBits; /* ヘッダサイズ(byte) */

	/* 画像サイズが異常の場合,エラーでリターン */
	if (x <= 0 || y <= 0) {
		return 0;
	}

	/* 出力ストリーム異常の場合,エラーでリターン */
	if (s == NULL || ferror(s)) {
		return 0;
	}

	/* 色数を計算 */
	if (c == 24) {
		color = 0;
	} else {
		color = 1;
		for (i=1;i<=c;i++) {
			color *= 2;
		}
	}

	/* ヘッダサイズ(byte)を計算 */
	/* ヘッダサイズはビットマップファイルヘッダ(14) + ビットマップ情報ヘッダ(40) + 色数 */
	bfOffBits = 14 + 40 + 4 * color;

	/* ビットマップファイルヘッダ(計14byte)を書出 */
	/* 識別文字列 */
	fputs("BM", s);

	/* bfSize ファイルサイズ(byte) */
	fputc4LowHigh(bfOffBits + (unsigned long)x * y, s);

	/* bfReserved1 予約領域1(byte) */
	fputc2LowHigh(0, s);

	/* bfReserved2 予約領域2(byte) */
	fputc2LowHigh(0, s);

	/* bfOffBits ヘッダサイズ(byte) */
	fputc4LowHigh(bfOffBits, s);

	/* ビットマップ情報ヘッダ(計40byte) */
	/* biSize 情報サイズ(byte) */
	fputc4LowHigh(40, s);

	/* biWidth 画像Xサイズ(dot) */
	fputc4LowHigh(x, s);

	/* biHeight 画像Yサイズ(dot) */
	fputc4LowHigh(y, s);

	/* biPlanes 面数 */
	fputc2LowHigh(1, s);

	/* biBitCount 色ビット数(bit/dot) */
	fputc2LowHigh(c, s);

	/* biCompression 圧縮方式 */
	fputc4LowHigh(0, s);

	/* biSizeImage 圧縮サイズ(byte) */
	fputc4LowHigh(0, s);

	/* biXPelsPerMeter 水平解像度(dot/m) */
	fputc4LowHigh(0, s);

	/* biYPelsPerMeter 垂直解像度(dot/m) */
	fputc4LowHigh(0, s);

	/* biClrUsed 色数 */
	fputc4LowHigh(0, s);

	/* biClrImportant 重要色数 */
	fputc4LowHigh(0, s);

	/* 書出失敗ならエラーでリターン */
	if (ferror(s)) {
		return 0;
	}

	/* 成功でリターン */
	return 1;
}

/*
	fputc2LowHigh 2バイトデータ書出(下位?上位)
	
	2バイトのデータを下位?上位の順で書き出す

	●戻り値
		int:EOF…失敗, EOF以外…成功
	●引数
		unsigned short d:[i] データ
		FILE *s:[i] 出力ストリーム
*/
int fputc2LowHigh(unsigned short d, FILE *s)
{
	putc(d & 0xFF, s);
	return putc(d >> CHAR_BIT, s);
}

/*
	fputc4LowHigh 4バイトデータ書出(下位?上位)
	
	4バイトのデータを下位?上位の順で書き出す

	●戻り値
		int:EOF…失敗, EOF以外…成功
	●引数
		unsigned long d:[i] データ
		FILE *s:[i] 出力ストリーム
*/
int fputc4LowHigh(unsigned long d, FILE *s)
{
	putc(d & 0xFF, s);
	putc((d >> CHAR_BIT) & 0xFF, s);
	putc((d >> CHAR_BIT * 2) & 0xFF, s);
	return putc((d >> CHAR_BIT * 3) & 0xFF, s);
}
// ------------------------------------------------------------------
//                          END OF PROGRAM
// ------------------------------------------------------------------
