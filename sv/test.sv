// sv/test.sv
`include "uvm_pkg.sv"
`include "env.sv"
`include "seq_item.sv"

class alu_uvm_test extends uvm_test;
  alu_env env;

  `uvm_component_utils(alu_uvm_test)
  function new(string name, uvm_component parent); super.new(name,parent); endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    env = alu_env::type_id::create("env", this);
  endfunction

  task run_phase(uvm_phase phase);
    // Connect to agent
    fork
      begin
        @(posedge 1ns); // small delay
        env.trans.connect_to_agent("127.0.0.1", 5000);
      end
    join_none

    // Example: form batches and send
    int BATCH = 4;
    int i;
    bit [7:0] a = 1, b = 2;
    for (int batch_i=0; batch_i<5; batch_i++) begin
      // build JSON array of observations
      string json = "[";
      for (i=0;i<BATCH;i++) begin
        int seqid = batch_i*BATCH + i;
        json = {json, $sformatf("{"type":"obs","seq_id":%0d,"a":%0d,"b":%0d,"last_result":%0d}", seqid, a+i, b+i, 0)};
        if (i != BATCH-1) json = {json, ","};
      end
      json = {json, "]"};
      `uvm_info("TEST",$sformatf("Sending batch JSON: %s", json), UVM_LOW)
      env.trans.send_obs_batch_and_publish(json);

      // give some time for driver to process
      #100ns;
      a = a + 5;
      b = b + 3;
    end

    env.trans.close();
  endtask
endclass
