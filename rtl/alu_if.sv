// rtl/alu_if.sv
interface alu_if (input bit clk, input bit rst_n);
  logic [7:0] a;
  logic [7:0] b;
  logic [2:0] op;   // 0:add,1:sub,2:and,3:or,4:xor
  logic        start;
  logic        done;
  logic [7:0]  result;
  logic        err;

  modport DUT (input a,b,op,start, output done,result,err);
  modport TB  (output a,b,op,start, input done,result,err);
endinterface
