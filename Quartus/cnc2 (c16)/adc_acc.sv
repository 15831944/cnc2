`ifndef _adc_acc_
`define _adc_acc_

`include "header.sv"
`include "adc_MCP3008.sv"
`include "cic_decim_osr.sv"
`include "accum.sv"
`include "adc_permit.sv"

module adc_acc #(parameter
	CLOCK_HZ = 72_000_000,
	SCLK_HZ = 1_000_000,
	ADC_WIDTH = 10
)(
	input clk, aclr, sclr,
	
	// SPI
	output sclk, csn, mosi,
	input miso,
	
	output [7:0][ADC_WIDTH-1:0] adc,
	output [7:0] err,
	
	input soft_permit, fb_ena,
	input [ADC_WIDTH-1:0] low, high, // low < high
	
	input oi,
	
	output permit,
	
	input [1:0] ctr_state,
	output ctr_stop
);

	wire [7:0][ADC_WIDTH-1:0] chip_adc;
	wire [7:0] sample;
	
	// 6,579 Hz
	adc_MCP3008 #(.CLOCK_HZ(CLOCK_HZ), .SCLK_HZ(SCLK_HZ)) adc_inst(
		.clk, .aclr, .sclr,
		.sclk, .csn, .mosi, .miso,
		.adc(chip_adc), .err, .sample
	);

	assign adc[1] = chip_adc[1];
	assign adc[2] = chip_adc[2];
	assign adc[3] = chip_adc[3];
	assign adc[4] = chip_adc[4];
	assign adc[5] = chip_adc[5];
	assign adc[6] = chip_adc[6];
	assign adc[7] = chip_adc[7];	
		
	adc_permit #(.ADC_WIDTH(ADC_WIDTH)) permit_inst(
		.clk, .aclr, .sclr,
		.adc(chip_adc[0]), .adc_err(err[0]), .adc_valid(sample[0]),
		.flt_adc(adc[0]), .flt_valid(),
		.soft_permit, .fb_ena, .low, .high,	
		.oi,
		.permit,
		.ctr_state, .ctr_stop
	);
		
endmodule :adc_acc

`endif // _adc_acc_
