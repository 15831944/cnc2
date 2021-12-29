timeunit 1ns;
timeprecision 1ps;

`include "SN74HC595.sv"

module piso_tb;

	localparam WIDTH = 16;
	localparam CLK_DIV = 10;

	bit clk = 0, aclr = 1, sclr = 0;
	bit [WIDTH-1:0] data = 0;
	bit wrreq = 0;
	wire busy, loaded;
	wire [WIDTH-1:0] shift;
	wire sclk, sdo, lock, sdi;

	wire [15:0] q;
	
	always #6.944ns clk++;
	
	initial begin
		repeat(10) @(posedge clk);
		aclr = 0;
		sclr = 0;
		
		repeat(10) @(posedge clk);
		
		wait(!busy) @(posedge clk);
		repeat(10) @(posedge clk);
		
		write(16'h8001);
		wait(!busy) @(posedge clk);		
		repeat(10) @(posedge clk);
		
		write(16'h1234);
		wait(!busy) @(posedge clk);		
		repeat(10) @(posedge clk);
		
		$stop(2);
	end
	
	always #100us $stop(2);
	
	piso #(.WIDTH(WIDTH), .CLK_DIV(CLK_DIV)) dut(.*);
	
	wire d_out7;
	
	SN74HC595 SN74HC595_inst0(
		.oen(1'b0), .rclk(lock), .srclrn(1'b1), .srclk(sclk), .ser(sdo),
		.q(q[7:0]), .d_out7(d_out7)
	);
	
	SN74HC595 SN74HC595_inst1(
		.oen(1'b0), .rclk(lock), .srclrn(1'b1), .srclk(sclk), .ser(d_out7),
		.q(q[15:8]), .d_out7(sdi)
	);
	
	task write(bit [15:0] dato);
		wait(!busy)
			@(posedge clk);
		
		data = dato;
		wrreq = 1;		
		@(posedge clk);
		
		wrreq = 0;
		@(posedge clk);
	endtask
	
endmodule :piso_tb
