// sv/tb_top.sv
`timescale 1ns/1ps
`include "../rtl/alu_if.sv"
`include "../rtl/alu_dut.sv"
`include "uvm_pkg.sv"
`include "test.sv"

module tb_top;
  bit clk;
  bit rst_n;

  alu_if u_if(.clk(clk), .rst_n(rst_n));
  alu_dut dut(.clk(clk), .rst_n(rst_n),
              .a(u_if.a), .b(u_if.b), .op(u_if.op), .start(u_if.start),
              .done(u_if.done), .result(u_if.result), .err(u_if.err));

  // clock/reset
  initial begin clk = 0; forever #5 clk = ~clk; end
  initial begin rst_n = 0; #20 rst_n = 1; end

  initial begin
    // Provide the virtual interface to UVM config DB for components
    uvm_config_db#(virtual alu_if)::set(null, "*", "vif", u_if);
    run_test("alu_uvm_test");
  end
endmodule
