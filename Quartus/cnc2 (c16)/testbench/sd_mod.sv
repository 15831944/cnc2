`ifndef _sd_mod_
`define _sd_mod_

// Sigma-Delta Modulator
// Input range -VREF...+VREF
module sd_mod #(
	parameter WIDTH = 16,
	parameter signed [WIDTH-1:0] VREF = {1'b0, {(WIDTH-1){1'b1}}}
)(
	input aclr,
	input bit signed [WIDTH-1:0] V,
	input clock,
	output sdo
);
	bit signed [16:0] sum = 0, acc = 0;

	always @(posedge clock, posedge aclr)
		if (aclr)
			acc <= V;
		else
			acc <= acc + sum;
	
	assign sdo = acc >= 0;
	
	always_comb
		if (sdo)
			sum = V - VREF;
		else
			sum = V + VREF;
	
endmodule :sd_mod

module sd_mods #(parameter
	NUM = 16,
	WIDTH = 16,
	bit [WIDTH-1:0] VREF = {1'b0, {(WIDTH-1){1'b1}}}
)(
	input aclr,
	input bit [NUM-1:0][WIDTH-1:0] V,
	input clock,
	output [NUM-1:0] sdo
);

	genvar i;
	
	generate for (i = 0; i < NUM; i++)
		begin
			sd_mod #(.VREF(VREF)) sd_mod_inst(
				.aclr, .V(V[i]),
				.clock, .sdo(sdo[i])
			);
		end
	endgenerate
	
endmodule :sd_mods

`endif
