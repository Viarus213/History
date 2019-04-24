module ROM_MODULE(addr, memory_out);

	input [3:0] addr;
	
	output reg [7:0] memory_out;
	
	
	always@(*) begin
		case(addr)
			4'b0000: memory_out <= 8'b00001111;
			4'b0001: memory_out <= 8'b10001010;
			4'b0010: memory_out <= 8'b00010001;
			4'b0011: memory_out <= 8'b00000101;
			4'b0100: memory_out <= 8'b10011010;
			4'b0101: memory_out <= 8'b01011110;
			4'b0110: memory_out <= 8'b01101110;
			4'b0111: memory_out <= 8'b01111111;
			4'b1000: memory_out <= 8'b10001110;
			4'b1001: memory_out <= 8'b10010001;
			4'b1010: memory_out <= 8'b10101010;
			4'b1011: memory_out <= 8'b10110101;
			4'b1100: memory_out <= 8'b11000101;
			4'b1101: memory_out <= 8'b11010011;
			4'b1110: memory_out <= 8'b11101111;
			default: memory_out <= 8'b00000000;
		endcase
	end
	
endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module Preskaler(CLK, RESET, CE, Q_preskaler, CEO_preskaler, p0, p1);

	input CLK, RESET, CE;
	
	output CEO_preskaler;
	output reg [27:0] Q_preskaler;
	
	
	//Frequency prescaler
	always @(posedge CLK or posedge RESET) begin
		if(RESET)
			Q_preskaler <= 28'd0;
		else begin
			if(CE) begin
				if(Q_preskaler != 28'd5)
					Q_preskaler <= Q_preskaler + 1;
				else
					Q_preskaler <= 28'd0;
			end
		end
	end
	 
	assign CEO_preskaler = CE & (Q_preskaler == 28'd5);
	
	
	//impulse generator: 
		//p0 = 1: next ROM address setting
		//p1 = 1: load accumulator
				
		
	output reg p0, p1; 
	reg [1:0] p;
	initial p = 2'b00; 
		
	always@(posedge CLK) begin
		case(p)
			2'b00: begin
				p0 = 1'b0;
				p1 = 1'b0;
				if(CEO_preskaler) begin
					p = 2'b01;
				end
			end
			2'b01: begin
				p0 = 1'b0;
				if(CEO_preskaler) begin
					p1 = 1'b1;
					p = 2'b10;
				end
			end
			2'b10: begin
				p1 = 1'b0;
				if(CEO_preskaler) begin
					p0 = 1'b1;
					p = 2'b11;
				end
			end
			2'b11: begin
				p0 = 1'b0;
				p1 = 1'b0;
				if(CEO_preskaler) begin
					p = 2'b00;
				end
			end
			default: begin
				p0 = 1'b0;
				p1 = 1'b1;
				p = 2'b00;
			end
		endcase
	end
	
endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module Memory_counter(CLK, RESET, CE, Q_mem_cnt); //ROM control

	input CLK, RESET, CE;
	
	output reg [3:0]Q_mem_cnt;

	
	always @(posedge CLK or posedge RESET) begin
		if(RESET)
			Q_mem_cnt <= 4'd0;
		else begin
			if(CE) begin
				if(Q_mem_cnt != 4'd14)
					Q_mem_cnt <= Q_mem_cnt + 1; 
				else
					Q_mem_cnt <= 4'd0;
			end
		end
	end
	
endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module TOP_ROM(CLK, RESET, douta, p1);

	input CLK, RESET;
	
	output [7:0]douta;
	output p1;
	
	wire p0;
	wire [3:0]Q_mem_cnt;

	
	//module ROM_MODULE(addr, memory_out);
		ROM_MODULE ROM_1(Q_mem_cnt, douta);
		
	//module Preskaler(CLK, RESET, CE, Q_preskaler, CEO_preskaler, p0, p1);
		Preskaler preskaler_1(CLK, RESET, 1'b1, , , p0, p1);
		
	//module Memory_counter(CLK, RESET, CE, Q_mem_cnt);
		Memory_counter counter_1(CLK, RESET, p0, Q_mem_cnt);

endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module register_4bit(CLK, RESET, CE, in, out);
	input CLK, RESET, CE;
	input [3:0] in;
	
	output reg [3:0] out;
	
	
	always@(posedge CLK or posedge RESET) begin
		if(RESET)
			out <= 4'b0000;
		else begin
			if(CE)
				out <= in;
		end
	end
	
endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Input signal distribution
module distribution(signal_in, address, signal_out);
	input [7:0] signal_in;
	
	output reg [3:0] address;		//Operation code
	output reg [3:0] signal_out;	//Data
	
	always@(signal_in) begin
		address = signal_in[7:4];
		signal_out = signal_in[3:0];
	end
		
endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module operation(address, accumulator, register, out);
	input [3:0] address;		//Operation code
	input [3:0] register;		//Signal from ROM
	input [3:0] accumulator;	//Signal from accumulator
	
	output reg [3:0] out;		//Output
	
	
	always@(*) begin
		case(address)		
			4'b0000: out <= register;									//accumulator = register
			4'b0001: out <= accumulator;								//show whats in accumulator
			4'b0010: out <= !accumulator;								//logical negation
			4'b0011: out <= ~accumulator;								//bitwise negation
			4'b0100: out <= accumulator 	<< 1;						//left shift
			4'b0101: out <= accumulator	>> 1;						//right shift
			4'b0110: out <= accumulator 	+ 	1;						//increment
			4'b0111: out <= accumulator 	- 	1;						//decrement
			4'b1000: out <= accumulator 	+ 	register;			//addition
			4'b1001: out <= accumulator 	- 	register;			//subtraction
			4'b1010: out <= accumulator 	&& register;			//logical and
			4'b1011: out <= accumulator 	|| register;			//logical or
			4'b1100: out <= accumulator 	& 	register;			//bitwise and
			4'b1101: out <= accumulator 	| 	register;			//bitwise or
			4'b1110: out <= accumulator 	^ 	register;			//bitwise xor
			default: out <= 4'd0;
		endcase
	end
	
endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module input_mux(in_signal, in_switch, select, out_mux);
	input select;										//user or ROM
	input [3:0] in_signal;								//input from file
	input [3:0] in_switch;								//input from user
	
	output [3:0] out_mux;
	
	
	assign out_mux = select ? in_signal : in_switch;	//0 - in_switch, 1 - in_signal
	
endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module ALU_module(CLK, RESET, CE, in, select_input, in_switch, accumulator_out);
	input CLK, RESET, CE;
	input select_input;						//Select input (from switches or file)
	input [7:0] in;							//Input signal
	input [3:0] in_switch;					//Switches input
	
	output [3:0] accumulator_out;
	
	wire [3:0] address;						//Operation code
	wire [3:0] register;					//Accumulator and register blocks
	wire [3:0] out_mux;						//Output of mux which signal
	wire [3:0] operation_out;
	
	
	//module distribution(signal_in, address, signal_out);
		distribution signal_distribution(in, address, register);							//block that distribue input signal: address and ALU signal
		
	//module register_4bit(CLK, RESET, CE, in, out);
		register_4bit accumulator_block(CLK, RESET, CE, operation_out, accumulator_out);	//accumulator
	
	//module operation(address, accumulator, register, out);
		operation operation_block(address, accumulator_out, out_mux, operation_out);		//block of operations
	
	//module input_mux(in_signal, in_switch, select, out_mux)
		input_mux signal_choice(register, in_switch, select_input, out_mux);				//signal from switches or from file

endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

module processor(CLK, RESET, select_input, in_switch, final_output, douta, bufor);

	input CLK, RESET;
	input select_input;						//Select input (from switches or file)
	input [3:0] in_switch;					//Switches input
	
	output [3:0] final_output;				//ALU output - after register
	output [7:0] douta;
	output [3:0] bufor;
	
	wire p1;

	
	assign bufor = 4'd0;
	
	//module TOP_ROM(CLK, RESET, douta, p1);
	TOP_ROM ROM_for_VGA(CLK, RESET, douta, p1);
	
	//module ALU_module(CLK, RESET, CE, in, select_input, in_switch, accumulator_out);
	ALU_module alu(CLK, RESET, p1, douta, select_input, in_switch, final_output);
	
endmodule