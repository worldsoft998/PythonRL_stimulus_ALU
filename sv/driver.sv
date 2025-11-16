// sv/driver.sv (driver receives items via analysis_imp and drives the interface)
`include "uvm_pkg.sv"
`include "seq_item.sv"
`include "../rtl/alu_if.sv"

class alu_driver extends uvm_component;
  virtual alu_if vif;

  // analysis imp to receive items published by transactor
  uvm_analysis_imp #(alu_seq_item, alu_driver) analysis_imp;

  `uvm_component_utils(alu_driver)
  function new(string name, uvm_component parent);
    super.new(name,parent);
    analysis_imp = new("analysis_imp", this, this);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    if (!uvm_config_db#(virtual alu_if)::get(this, "", "vif", vif))
      `uvm_fatal("NOVIF","Virtual interface not set");
  endfunction

  // the write callback from analysis_imp
  function void write(alu_seq_item trans_item);
    // drive the DUT with the received item (non-blocking, immediate in this PoC)
    `uvm_info("DRV", $sformatf("Driver received item op=%0d a=%0d b=%0d", trans_item.op, trans_item.a, trans_item.b), UVM_LOW)
    @(posedge vif.clk);
    vif.a <= trans_item.a;
    vif.b <= trans_item.b;
    vif.op <= trans_item.op;
    vif.start <= 1;
    @(posedge vif.clk);
    vif.start <= 0;
    wait (vif.done == 1);
    `uvm_info("DRV", $sformatf("Result: %0d err=%0d", vif.result, vif.err), UVM_LOW)
  endfunction
endclass
