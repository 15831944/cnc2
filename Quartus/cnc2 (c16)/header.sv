`ifndef _header_
`define _header_

`define VER_TYPE (2'h1) // 0 - stable, 1 - alpha, 2 - beta
`define FAC_VER (6'h0)
`define FAC_REV (8'h2) // CNC2
`define VER (8'h2)
`define REV (8'hE)

// 2.9 - red semaphore enable at start todo: add flash access
// 2.10 - adding a pause after HV is enabled, block TO timer
// 2.11 - adding a filter for a cut permit
// 2.12 - CIC filter
// 2.14 - if 13 for C16

`define INT16_MIN (16'sh8000)
`define INT16_MAX (16'sh7FFF)

`define GET_WIDTH(n) ( (2**$clog2(n) >= n) ? $clog2(n) : $clog2(n) + 1 )

`define TS_WIDTH	(32)
`define N_WIDTH	(32)
`define T_WIDTH	(32)

`define T_MIN		(`T_WIDTH'(100))
`define T_MAX		(`T_WIDTH'({32{1'b1}}))

typedef struct packed {
	logic dir, mask;
	logic [`TS_WIDTH-1:0] T;
	logic [`N_WIDTH-1:0] N;
	logic [31:0] V;
	logic isV;
	logic id;
} PlsData;

typedef struct packed {
	logic dir, mask;
	logic [`TS_WIDTH-1:0] ts;
	logic [`N_WIDTH-1:0] N;
} MotorParam;

typedef struct packed {
	logic [`TS_WIDTH:0] ts;
	logic [7:0][`N_WIDTH:0] N; // signed
	logic [31:0] gpo;
} TaskParam;

`endif
