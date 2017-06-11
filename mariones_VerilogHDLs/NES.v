`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: Ehime University
// Engineer: Hiroki Nakahara
// 
// Create Date: 2015/07/30 23:54:28
// Design Name: MarioNES
// Module Name: top_VGA_ctrl
// Project Name: MarioNES_version3
// Target Devices: Artix100
// Tool Versions: Vivado 2015.1
// Description: 
//  マリオ専用NESのトップモジュール
// Dependencies: 
// 
// Revision:
// Revision 1.00 - Released
// Additional Comments:
//  このソースコードはみきゃんウェアです。投票よろしく！
//////////////////////////////////////////////////////////////////////////////////
module NES(
    input CLK100MHz,
    input BTNC,
    output VGA_HS,
    output VGA_VS,
    output [3:0]VGA_R,
    output [3:0]VGA_G,
    output [3:0]VGA_B,
    input [5:0]SW,
    input BTNU,
    input BTNR,
    input BTNL,
    input BTND,
    output [10:0]LED
    );
              
    wire [31:0]w_frame; // フレーム数
    wire [11:0]w_d; // NES本体からの画素データ（RGB１２ビット）
    wire [15:0]w_adr; // 画素位置(Y,X)
    wire w_we; // 画素書き込みイネーブル, バッファBRAMに書き込む
    
    wire [11:0]color; //バッファBRAMから読みだした画素データ
    wire [9:0]x; // X座標
    wire [9:0]y; // Y座標
    
    reg [1:0]CLK25MHz; // 25MHzはVGAコントローラ用
    wire CLK50MHz; // バッファBRAM用
    
    assign LED[7:0] = w_frame[7:0];
    
    // -----------------------------------------------------
    // VGAコントローラからの信号を出力
    // -----------------------------------------------------
    assign VGA_R = x < 10'd512 ? color[3:0]  : 4'b0;
    assign VGA_G = x < 10'd512 ? color[7:4]  : 4'b0;
    assign VGA_B = x < 10'd512 ? color[11:8] : 4'b0;

    // -----------------------------------------------------
    // generate 25MHz from 100MHz
    // -----------------------------------------------------
    always@( posedge CLK100MHz or posedge BTNC)begin
        if( BTNC == 1'b1)begin
            CLK25MHz <= 2'b00;
        end else begin
            CLK25MHz <= CLK25MHz + 1'b1;
        end
    end
    
    assign CLK50MHz = CLK25MHz[0];
    
    // -----------------------------------------------------
    // VGA controller
    // Nexys4のサンプルデザインから拝借しました
    // -----------------------------------------------------
    vga640x480 vga_ctrl_inst(
        .dclk( CLK25MHz[1]), //pixel clock: 25MHz
        .clr( BTNC),         //asynchronous reset
        .hsync( VGA_HS),     //horizontal sync out
        .vsync( VGA_VS),     //vertical sync out
        .red( ),    //red vga output
        .green( ),  //green vga output
        .blue( ),   //blue vga output
        .x( x),
        .y( y)
    );
    
    // ------------------------------------------------------
    // NES制御信号の生成
    // フレーム描画開始時に, 
    // バッファBRAMの最上位ビット(NES_buffer_page)を切り替えて
    // VGAコントローラが読出し中に, もう一つのページに
    // NESからの描画データを書き込む
    // ------------------------------------------------------
    reg NES_ap_start; // NES開始信号. 動作中は1にアサートしておく
    reg NES_buffer_page; // バッファBRAMのページ番号
    
    wire NES_ap_done; // 1フレームのエミュレーション終了信号
    
    // state machine which exchanges the NES and the VGA Ctrl
    always@( posedge CLK100MHz or posedge BTNC)begin
        if( BTNC == 1'b1)begin
            NES_ap_start    <= 1'b0;
            NES_buffer_page <= 1'b0;
        end else begin
            if( NES_ap_start == 1'b1)begin
                if( NES_ap_done == 1'b1)begin
                    NES_ap_start <= 1'b0;
                end
            end else begin
                if( VGA_HS == 1'b1 && VGA_VS == 1'b1)begin
                    NES_ap_start <= 1'b1;
                    NES_buffer_page <= ~NES_buffer_page;
                end
            end
        end
    end
    
    assign LED[10:8] = {NES_ap_start, NES_buffer_page, NES_ap_done};

    // ------------------------------------------------------------------    
    // バッファBRAM, ping-pongメモリ構造なのででかいねぇ…
    // パレット番号だけを保持して, モニタに描画時に
    // 小さいテーブルを引けば半分にできた…
    // ------------------------------------------------------------------    
    blk_mem_gen_0 buffer_mem(
      .clka( CLK50MHz),    // input wire clka
      .ena( 1'b1),      // input wire ena
      .wea( w_we),      // input wire [0 : 0] wea
      .addra( { ~NES_buffer_page, w_adr[7:0], w_adr[15:8]}),  // input wire [16 : 0] addra
      .dina( w_d),    // input wire [5 : 0] dina
      .clkb( CLK25MHz),    // input wire clkb
      .enb( 1'b1),      // input wire enb
      .addrb( { NES_buffer_page, x[8:1], y[8:1]}),  // input wire [16 : 0] addrb
      .doutb( color)  // output wire [5 : 0] doutb
    );
    
    // ------------------------------------------------------------------
    // NES for super mario brothers only
    // Vivado HLSから生成
    // ------------------------------------------------------------------
     mariones_top mariones_inst(
           .ap_clk(CLK25MHz),
           .ap_rst(BTNC),
           .ap_start( NES_ap_start | SW[0]),
           .ap_done( NES_ap_done),
           .ap_idle(),
           .ap_ready(),
           .reset( SW[1]),
           .bmp_address0( w_adr),
           .bmp_ce0(),
           .bmp_we0( w_we),
           .bmp_d0( w_d),
           //.pad0({JC2,JC1,JC4,JC3,BTNU,BTNR,JC7,JC8}), ← PMod C端子からゲームパッドをつなぐ時の一例.
           .pad0({ BTNR,BTNL,BTND,BTNU,SW[5:2]}), //{ Right, Left, Down, Up, Start, Select, B, A}
           .pad1(8'b0), // 2プレイヤのパッド信号
           .ap_return( w_frame)
   );
   
endmodule
// --------------------------------------------------------------------------
//                             END OF SOURCE CODE
// --------------------------------------------------------------------------
