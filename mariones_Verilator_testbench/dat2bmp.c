#include<stdio.h>
#include<stdlib.h>

#define PIXEL_NUM_X (256)	/* 画像のXサイズ */
#define PIXEL_NUM_Y (240)	/* 画像のYサイズ */
#define COLOR_BIT (24)	/* 色ビット */
#define PIC_DATA_SIZE (PIXEL_NUM_X * 3 * PIXEL_NUM_Y)	/* bitmapのサイズ */
#define CHAR_BIT 8

int putBmpHeader(FILE *s, int x, int y, int c);
int fputc4LowHigh(unsigned long d, FILE *s);
int fputc2LowHigh(unsigned short d, FILE *s);

int main( int argc, char *argv[])
{
	FILE *fp_bmp;
	// output BMP file
	char fname[256], line[256];
	unsigned char *bmp;
	int x, y, dat;
		
	if( argc < 1)
		fprintf( stderr, "USAGE: dat2bmp [text file]\n");
	
	printf("input file=%s\n", argv[1]);
	
	// bitmap用メモリを確保	
   bmp = (unsigned char *)malloc(PIC_DATA_SIZE * sizeof(unsigned char));
   if( bmp == NULL){
		fprintf( stderr, "CAN'T MALLOC *bmp\n");
	}
	 
	// read log file
	sprintf(fname, "%s", argv[1]);
	fp_bmp = fopen( fname, "r");
	if( fp_bmp == NULL){
		fprintf( stderr, "CAN'T OPEN %s\n", fname);
	}
	
	while( fgets( line, 256, fp_bmp) != NULL){
		sscanf( line, "X= %d Y= %d %x\n", &x, &y, &dat);
		//printf("x=%d y=%d %x\n", x, y, dat);
		bmp[ (239 - y) * 256 * 3 + x * 3]     = (dat >> 16) & 0xFF;
    	bmp[ (239 - y) * 256 * 3 + x * 3 + 1] = (dat >> 8) & 0xFF;
    	bmp[ (239 - y) * 256 * 3 + x * 3 + 2] = dat & 0xFF;
    	/*
    	bmp[ (239 - y) * 256 * 3 + x * 3]     = 0xFF;
    	bmp[ (239 - y) * 256 * 3 + x * 3 + 1] = 0xFF;
    	bmp[ (239 - y) * 256 * 3 + x * 3 + 2] = 0xFF;
    	*/
	}
	
	
	fclose( fp_bmp);
	//exit(0);
	
	// write BMP file
	sprintf(fname, "%s.bmp", argv[1]);
	fp_bmp = fopen( fname, "wb");
	if( fp_bmp == NULL){
		fprintf( stderr, "CAN'T OPEN %s\n", fname);
	}
	
	putBmpHeader( fp_bmp, PIXEL_NUM_X, PIXEL_NUM_Y, COLOR_BIT);
	fwrite( bmp, sizeof(unsigned char), PIC_DATA_SIZE, fp_bmp);
	
	fclose( fp_bmp);
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


