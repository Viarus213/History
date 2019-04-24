`timescale 1ns / 1ps

module ALU_tb;

   reg CLK;
   reg RESET;
   reg select_input;
   reg [3:0] in_switch;
   reg [3:0] bufor;
   
   wire [7:0] douta;
   wire [3:0] final_output;

   
   processor test_bench (CLK, RESET, select_input, in_switch, final_output, douta, bufor);

   initial begin 
      CLK = 1'b0;
      RESET = 1'b1;
      select_input = 1'b1;
      in_switch = 4'd0;
   end
   
   always #5 CLK = !CLK; //clock generator
   
   initial begin
      #250 RESET = 1'b0;
      #3485 select_input = 1'b0;
      in_switch = 4'b0000;      //accumulator = register    
      #240 in_switch = 4'b0001; //show whats in accumulator
      #240 in_switch = 4'b0010; //logical negation
      #240 in_switch = 4'b0011; //bitewise negation
      #240 in_switch = 4'b0100; //left shift
      #240 in_switch = 4'b0101; //right shift
      #240 in_switch = 4'b0110; //increment
      #240 in_switch = 4'b0111; //decrement
      #240 in_switch = 4'b1000; //addition
      #240 in_switch = 4'b1001; //subtraction
      #240 in_switch = 4'b0000; //logical and
      #240 in_switch = 4'b0000; //logical or
      #240 in_switch = 4'b1100; //bitewise and
      #240 in_switch = 4'b1101; //bitewise or
      #240 in_switch = 4'b1110; //bitewise xor
      #240 select_input = 4'd1;
   end
endmodule
