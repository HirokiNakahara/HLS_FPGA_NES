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
//  �}���I��pNES�̃g�b�v���W���[��
// Dependencies: 
// 
// Revision:
// Revision 1.00 - Released
// Additional Comments:
//  ���̃\�[�X�R�[�h�݂͂����E�F�A�ł��B���[��낵���I
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
              
    wire [31:0]w_frame; // �t���[����
    wire [11:0]w_d; // NES�{�̂���̉�f�f�[�^�iRGB�P�Q�r�b�g�j
    wire [15:0]w_adr; // ��f�ʒu(Y,X)
    wire w_we; // ��f�������݃C�l�[�u��, �o�b�t�@BRAM�ɏ�������
    
    wire [11:0]color; //�o�b�t�@BRAM����ǂ݂�������f�f�[�^
    wire [9:0]x; // X���W
    wire [9:0]y; // Y���W
    
    reg [1:0]CLK25MHz; // 25MHz��VGA�R���g���[���p
    wire CLK50MHz; // �o�b�t�@BRAM�p
    
    assign LED[7:0] = w_frame[7:0];
    
    // -----------------------------------------------------
    // VGA�R���g���[������̐M�����o��
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
    // Nexys4�̃T���v���f�U�C������q�؂��܂���
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
    // NES����M���̐���
    // �t���[���`��J�n����, 
    // �o�b�t�@BRAM�̍ŏ�ʃr�b�g(NES_buffer_page)��؂�ւ���
    // VGA�R���g���[�����Ǐo������, ������̃y�[�W��
    // NES����̕`��f�[�^����������
    // ------------------------------------------------------
    reg NES_ap_start; // NES�J�n�M��. ���쒆��1�ɃA�T�[�g���Ă���
    reg NES_buffer_page; // �o�b�t�@BRAM�̃y�[�W�ԍ�
    
    wire NES_ap_done; // 1�t���[���̃G�~�����[�V�����I���M��
    
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
    // �o�b�t�@BRAM, ping-pong�������\���Ȃ̂łł����˂��c
    // �p���b�g�ԍ�������ێ�����, ���j�^�ɕ`�掞��
    // �������e�[�u���������Δ����ɂł����c
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
    // Vivado HLS���琶��
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
           //.pad0({JC2,JC1,JC4,JC3,BTNU,BTNR,JC7,JC8}), �� PMod C�[�q����Q�[���p�b�h���Ȃ����̈��.
           .pad0({ BTNR,BTNL,BTND,BTNU,SW[5:2]}), //{ Right, Left, Down, Up, Start, Select, B, A}
           .pad1(8'b0), // 2�v���C���̃p�b�h�M��
           .ap_return( w_frame)
   );
   
endmodule
// --------------------------------------------------------------------------
//                             END OF SOURCE CODE
// --------------------------------------------------------------------------
