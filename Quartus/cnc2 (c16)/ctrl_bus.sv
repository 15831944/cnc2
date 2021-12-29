`ifndef _ctrl_bus_
`define _ctrl_bus_

`include "header.sv"

//`include "sipo.sv"
`include "piso.sv"
`include "keyboard.sv"
`include "lpf_cap.sv"
`include "my_types.sv"
`include "hv_enabled.sv"

module ctrl_bus #(parameter
	BAR = 'h0,
	MASK = 'h3F
)(
	input clk, aclr,
	output reg sclr,
	
	input [15:0] rdaddr, wraddr,
	input [1:0] be,
	input write,
	input [15:0] wrdata,
	output reg [15:0] rddata,
	output reg nirq,
	
	// Outputs
	output [15:0] sig_out,
	output sig_oe_n,
	
	// Generator serial output
	output gen_sclk, gen_sdo, gen_lock,
	input gen_sdi,
	
	// Pult serial output
	output pult_sclk, pult_sdo, pult_lock,
	input pult_sdi,
	
	// Limit switches
	input mtr_permit, mtr_timeout, ctr_stop, // timeout local signal
	input power_OK, wire_break,
	input [7:0] sig_in,	
	
	output alarm, // limit switch alarm
	output reg led,
	
	output reg [2:0] sem,
	output reg sem_ena,
	
	output hv_enabled
);

//	localparam bit [9:0] DEFAUL_INPUT_LEVEL = 10'h2_08; // reverse: power_OK and wire_ctrl
	localparam bit [9:0] DEFAUL_INPUT_LEVEL = 10'h0_00;

	wire rdhit = (rdaddr & ~MASK) == BAR;
	wire wrhit = (wraddr & ~MASK) == BAR;
	
	wire [15:0] l_rdaddr = rdaddr & MASK[15:0];
	wire [15:0] l_wraddr = wraddr & MASK[15:0];
	
	wire [4:0] flag;
	reg [4:0] irq_mask = '0;
	
	localparam bit [15:0] SIG_OUT_CLR = 16'd3<<8 | 16'h3F<<2;
	localparam bit [15:0] LOCK_SIG_OUT_CLR = ~(16'h7<<10);
	sig_out_t sig_out_req = '0, sigo_reg = '0;
	reg sig_out_ena = 1'b0;
	
	localparam bit [15:0] GEN_DEFAULT = 16'd8<<8 | 16'd36<<0;	
	reg [15:0] gen_out = GEN_DEFAULT, gen_reg = GEN_DEFAULT;
	wire [15:0] gen_old;
	
	pult_t pult_dato_reg = '0, pult_dato = '0; // all leds off
	wire [15:0] pult_old;
	
	// Limit Switches
	wire [9:0] key_in, key;
	reg [9:0] key_level = DEFAUL_INPUT_LEVEL, key_reg = '0, key_down = '0;
	wire [9:0] key_down_clr;
	reg lock = 1'b1;
	
	wire [9:0] flt_key, flt_ready, flt_timeout;
	wire flt_allready;
	
	reg soft_alarm = 1'b0;
	reg [5:0] limsw_mask = '1, limsw_flag = {1'b1, 5'h0};
	wire [5:0] limsw, limsw_clr;
//	lim_switch_t limsw_mask = '{default:'1}, limsw_flag = '{default:'0};
//	lim_switch_t limsw, limsw_clr;
	wire alarm_clk;
	
	reg power_on_flag = 1'b1, pll_reset_flag = 1'b1, mcu_flag = 1'b0;
	
	localparam bit [15:0] HV_ENA_PRESCALE = 16'(7_200 - 1); // 0.1 ms
	localparam bit [15:0] HV_ENA_LENGTH = 16'(50_000 - 1); // 5 sec
	reg [15:0] hv_ena_prescale, hv_ena_length;

	task reset();
		soft_alarm <= 1'b0;
		limsw_mask <= '1;
		led <= 1'b0;
		sem_ena <= 1'b0;
		hv_ena_prescale <= HV_ENA_PRESCALE;
		hv_ena_length <= HV_ENA_LENGTH;
	endtask
	
	always_ff @(posedge clk, posedge aclr)
		if (aclr)
			begin
				reset();
				pll_reset_flag <= 1'b1;
			end
		else if (sclr)
			reset();
		else if (write && wrhit)
			case (l_wraddr)
				'h2: if (be[0]) irq_mask <= wrdata[4:0];
				
				'h6: if (be[0])
							begin
								if (wrdata[0] == 1'b0) power_on_flag <= 1'b0; // power on
								if (wrdata[1] == 1'b0) pll_reset_flag <= 1'b0; // pll restart
								if (wrdata[2] == 1'b1) mcu_flag <= 1'b1; // MCU started
							end
				'h8: if (be[0]) limsw_mask[4:0] <= wrdata[4:0]; // mask of limit switches
				
				'hC: if (be[0] && wrdata[5]) soft_alarm <= 1'b1; // set
				'hE: if (be[0] && wrdata[5]) soft_alarm <= 1'b0; // clear
				
				'h10:	begin
							if (be[0]) key_level[7:0] <= wrdata[7:0];
							if (be[1]) key_level[9:8] <= wrdata[9:8];
						end
						
				'h24:	begin
							if (be[0]) hv_ena_prescale[7:0] <= wrdata[7:0];
							if (be[1]) hv_ena_prescale[15:8] <= wrdata[15:8];
						end
				'h26:	begin
							if (be[0]) hv_ena_length[7:0] <= wrdata[7:0];
							if (be[1]) hv_ena_length[15:8] <= wrdata[15:8];
						end
				
				'h34: if (be[1]) sem_ena <= wrdata[8];
				
			
				'h3A: if (be[0]) led <= wrdata[0];
			endcase
	
	always_ff @(posedge clk, posedge aclr)
		if (aclr)
			begin
				sig_out_req <= '0;
				gen_out <= GEN_DEFAULT;				
				pult_dato_reg <= '0;
			end
		else if (sclr)
			begin
				sig_out_req <= '0;
				gen_out <= GEN_DEFAULT;				
				pult_dato_reg <= '0;
			end
		else if (lock)
			sig_out_req <= sig_out_req & ~LOCK_SIG_OUT_CLR;
		else if (alarm_clk)
			sig_out_req <= sig_out_req & ~SIG_OUT_CLR;
		else if (write && wrhit)
			case (l_wraddr)
				'h20:	begin
							if (be[0]) sig_out_req[7:0] <= wrdata[7:0];
							if (be[1])
								begin
									sig_out_req[8] <= wrdata[8] && !wrdata[9]; // fwd
									sig_out_req[9] <= wrdata[9] && !wrdata[8]; // rev
									sig_out_req[15:10] <= wrdata[15:10];
								end
						end						
				'h22:	begin
							if (be[0]) gen_out[7:0] <= wrdata[7:0];
							if (be[1]) gen_out[15:8] <= wrdata[15:8];
						end
				
				'h30:	begin
							if (be[0]) pult_dato_reg[7:0] <= wrdata[7:0];
							if (be[1]) pult_dato_reg[15:8] <= wrdata[15:8];
						end
			endcase
			
	always_ff @(posedge clk, posedge aclr)
		if (aclr)
			sem <= '0;
		else if (sclr)
			sem <= '0;
		else if (limsw[4:2] != '0)
			sem <= 3'b001; // red
		else if (write && wrhit)
			case (l_wraddr)
				'h34:	if (be[0]) sem <= wrdata[2:0];
			endcase
	
	reg sig_out_we = 1'b0, gen_out_we = 1'b0;
	reg new_key_level = 1'b0;
	
	assign limsw_clr = {6{write && wrhit && l_wraddr == 'hE && be[0]}} & wrdata[5:0];
	
	assign key_down_clr[7:0] = {8{write && wrhit && l_wraddr == 'h14 && be[0]}} & wrdata[7:0];
	assign key_down_clr[9:8] = {2{write && wrhit && l_wraddr == 'h14 && be[1]}} & wrdata[9:8];
	
	wire soft_alarm_clk = write && wrhit && l_wraddr == 'hC && be[0] && wrdata[5];
	
	always_ff @(posedge clk, posedge aclr) begin
		sclr			<= aclr ? 1'b1 : write && wrhit && l_wraddr == 'h4 && be[0] && wrdata[0];
		
		new_key_level	<= aclr ? 1'b0 : write && wrhit && l_wraddr == 'h10 && |be;
		
		sig_out_we	<= aclr ? 1'b0 : write && wrhit && l_wraddr == 'h20 && |be || sclr || alarm_clk;
		gen_out_we	<= aclr ? 1'b0 : write && wrhit && l_wraddr == 'h22 && |be || sclr || alarm_clk;
	end

	always_ff @(posedge clk, posedge aclr)
		if (aclr)
			rddata <= '0;
		else if (sclr)
			rddata <= '0;
		else if (rdhit)
			case (l_rdaddr)
				'h0: rddata <= 16'(flag);
				'h2: rddata <= 16'(irq_mask);
				
				'h6: rddata <= {13'h0, mcu_flag, pll_reset_flag, power_on_flag}; // testing				
				'h8: rddata <= 16'({1'b1, limsw_mask});
				
				'hC: rddata <= 16'(limsw);
				'hE: rddata <= 16'(limsw_flag);
				
				'h10: rddata <= 16'(key_level); // todo: mask
				'h12: rddata <= 16'(key);
				'h14: rddata <= 16'(key_down);
				'h16: rddata <= 16'(flt_timeout);

				'h20: rddata <= sig_out_req;
				'h22: rddata <= gen_out;
				'h24: rddata <= hv_ena_prescale;
				'h26: rddata <= hv_ena_length;
				
				'h2A: rddata <= gen_old;

				'h30: rddata <= pult_dato;
				'h32: rddata <= pult_old;
				'h34: rddata <= {7'h0, sem_ena, 5'h0, sem};
				
				'h3A: rddata <= {15'h0, led};
				'h3C: rddata <= {`VER, `REV};
				'h3E: rddata <= {`VER_TYPE, `FAC_VER, `FAC_REV};
				
				default: rddata <= '0;
			endcase
		else
			rddata <= '0;
	
	always_ff @(posedge clk, posedge aclr)
		if (aclr)
			sigo_reg <= '0;
		else if (sclr)
			sigo_reg <= '0;
		else if (lock)
			sigo_reg <= sigo_reg & ~LOCK_SIG_OUT_CLR;			
		else if (sig_out_we)
			sigo_reg <= sig_out_req;
			
	assign sig_out = sigo_reg;
			
	always_ff @(posedge clk, posedge aclr)
		if (aclr)
			gen_reg <= GEN_DEFAULT;
		else if (sclr)
			gen_reg <= GEN_DEFAULT;
		else if (gen_out_we)
			gen_reg <= gen_out;			
	
	always_ff @(posedge clk, posedge aclr)
		sig_out_ena <= aclr ? 1'b0 : (sclr ? 1'b0 : 1'b1);
		
	assign sig_oe_n = !sig_out_ena;

	always @(posedge clk)
		begin
			pult_dato.res <= pult_dato_reg.res;
			pult_dato.pump_ena <= sigo_reg.pump_ena;
			pult_dato.drum_ena <= sigo_reg.drum_fwd ^ sigo_reg.drum_rev;
	//		pult_dato.wire_ctrl <= limsw_mask.wire_ctrl;
			pult_dato.wire_ctrl <= limsw_mask[3];
			
			pult_dato.drum_vel <= 7'((8'h1 << sigo_reg.drum_vel) - 1'b1);
			pult_dato.ena <= sigo_reg.drum_vel != '0;
		end
	
	// SPI data output
	piso_always #(.WIDTH(2 * 8), .CLK_DIV(18)) piso_ind(
		.clk, .aclr, .sclr,
		.data(pult_dato),
		.data_old(pult_old),
		.sclk(pult_sclk), .sdo(pult_sdo), .lock(pult_lock), .sdi(pult_sdi)
	);
	
	piso_always #(.WIDTH(2 * 8), .CLK_DIV(18)) piso_ctrl(
		.clk, .aclr, .sclr,
		.data(gen_reg),
		.data_old(gen_old),
		.sclk(gen_sclk), .sdo(gen_sdo), .lock(gen_lock), .sdi(gen_sdi)
	);
	
	// Limit switches
	assign key_in = {!power_OK, wire_break, sig_in};
//	assign key_in = {1'b0, wire_break, sig_in};
	
	genvar k;
	generate for (k = 0; k < $bits(key_in); k++)
		begin :gen_limsw
			input_filter #(.POLL_CLOCK(250_000), .FILTER_WIDTH(8))
				flt_inst( .clock(clk), .aclr, .sclr(sclr || new_key_level), .in(key_in[k]), .level(key_level[k]), .out(flt_key[k]), .ready(flt_ready[k]), .timeout(flt_timeout[k]));
		end
	endgenerate

//	assign key[4] = flt_key[4] || !flt_ready[4] || flt_timeout[4];
//	assign key[5] = flt_key[5] || !flt_ready[5] || flt_timeout[5];
//	assign key[6] = flt_key[6] || !flt_ready[6] || flt_timeout[6];
//	assign key[7] = flt_key[7] || !flt_ready[7] || flt_timeout[7];

//	assign key[0] = (flt_key[0] || !flt_ready[0] || flt_timeout[0]) || key[4];
//	assign key[1] = (flt_key[1] || !flt_ready[1] || flt_timeout[1]) || key[5];
//	assign key[2] = (flt_key[2] || !flt_ready[2] || flt_timeout[2]) || key[6];
//	assign key[3] = (flt_key[3] || !flt_ready[3] || flt_timeout[3]) || key[7];

	assign key[0] = flt_key[4] || !flt_ready[4] || flt_timeout[4];
	assign key[1] = flt_key[5] || !flt_ready[5] || flt_timeout[5];
	assign key[2] = flt_key[6] || !flt_ready[6] || flt_timeout[6];
	assign key[3] = flt_key[7] || !flt_ready[7] || flt_timeout[7];
	assign key[7:4] = '0;
	
	assign key[8] = flt_key[8] || !flt_ready[8] || flt_timeout[8];
	assign key[9] = flt_key[9] || !flt_ready[9] || flt_timeout[9];
		
	always_ff @(posedge clk, posedge aclr)
		key_reg <= aclr ? '0 : (sclr || new_key_level) ? '0 : key;
	
	assign flt_allready = &flt_ready;
	
	genvar i;
	generate for (i = 0; i < $bits(key); i++)
		begin: key_gen
			always_ff @(posedge clk, posedge aclr)
				if (aclr)
					key_down[i] <= 1'b0;
				else if (sclr || new_key_level || !flt_allready)
					key_down[i] <= 1'b0;
				else if (!key_reg[i] && key[i]) // key down
					key_down[i] <= 1'b1;
				else if (key_down_clr[i])
					key_down[i] <= 1'b0;
		end
	endgenerate
	
	assign limsw = {
		soft_alarm,
		key[9], // power
		key[8], // wire
		key[0] || key[3], // alarm || limsw
		key[2], // rev
		key[1] // fwd
	};
	
	always @(posedge clk, posedge aclr)
		lock <= aclr ? 1'b1 : (sclr || new_key_level) ? 1'b1 : (limsw[4:2] & limsw_mask[4:2]) != '0; // alarm

//	always_comb begin
//		limsw.soft_alarm	= soft_alarm;
//		limsw.power_off	= key[17];
//		limsw.wire_ctrl	= key[16];
//		limsw.alarm			= key[0] || key[3];
//		limsw.drum_rev		= key[2];
//		limsw.drum_fwd		= key[1];
//	end
	
	genvar j;
	generate for (j = 0; j < $bits(limsw); j++)
		begin :gen_lim
			always_ff @(posedge clk, posedge aclr)
				if (aclr)
					limsw_flag[j] <= (j == $bits(limsw) - 1) ? 1'b1 : 1'b0; // soft alarm
				else if (sclr || new_key_level || !flt_allready || !limsw_mask[j])
					limsw_flag[j] <= (j == $bits(limsw) - 1) ? 1'b1 : 1'b0; // soft alarm - 1
				else if (limsw[j])
					limsw_flag[j] <= 1'b1;
				else if (limsw_clr[j])
					limsw_flag[j] <= 1'b0;
		end
	endgenerate
	
	assign alarm = limsw_flag != '0;
	
	reg alarm_reg = 1'b0;
	
	always_ff @(posedge clk, posedge aclr)
		alarm_reg <= (aclr) ? 1'b0 : (sclr || new_key_level) ? 1'b0 : alarm;
			
	assign alarm_clk = {alarm_reg, alarm} == 2'b01 || soft_alarm_clk; // CPU must work in alarm state
	
	assign flag = {|key_down, hv_enabled, mtr_timeout, !mtr_permit || ctr_stop, alarm};
	
	always_ff @(posedge clk, posedge aclr)
		nirq <= aclr ? 1'b1 : (sclr || new_key_level) ? 1'b1 : (flag & irq_mask) == '0;
	
	wire hv_ena = sig_out[0];
	wire [5:0] current = sig_out[7:2];
	
	hv_enabled hv_ena_inst(
		.clk, .aclr, .sclr,
		.hv(hv_ena && current != 0), .permit(mtr_permit),
		.prescale(hv_ena_prescale), .length(hv_ena_length),
		.enabled(hv_enabled)
	);
		
endmodule :ctrl_bus

`endif
