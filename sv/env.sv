// sv/env.sv
`include "uvm_pkg.sv"
`include "transactor.sv"
`include "driver.sv"

class alu_env extends uvm_env;
  alu_transactor trans;
  alu_driver drv;

  `uvm_component_utils(alu_env)
  function new(string name, uvm_component parent); super.new(name,parent); endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    trans = alu_transactor::type_id::create("trans", this);
    drv   = alu_driver::type_id::create("drv", this);
  endfunction

  function void connect_phase(uvm_phase phase);
    super.connect_phase(phase);
    // Provide vif handles
    uvm_config_db#(virtual alu_if)::set(this, "trans", "vif", trans.vif);
    uvm_config_db#(virtual alu_if)::set(this, "drv", "vif", drv.vif);
    // connect analysis port -> imp
    trans.ap.connect(drv.analysis_imp);
  endfunction
endclass
