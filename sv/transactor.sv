// sv/transactor.sv  (UVM transactor, batched)
`include "uvm_pkg.sv"
`include "../dpi/dpi_imports.sv"
`include "seq_item.sv"

class alu_transactor extends uvm_component;
  // analysis port to publish sequence items (driver subscribes)
  uvm_analysis_port #(alu_seq_item) ap;

  virtual alu_if vif;

  `uvm_component_utils(alu_transactor)
  function new(string name, uvm_component parent);
    super.new(name,parent);
    ap = new("ap", this);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
    if (!uvm_config_db#(virtual alu_if)::get(this, "", "vif", vif))
      `uvm_fatal("NOVIF","Virtual interface not set");
  endfunction

  task connect_to_agent(string host="127.0.0.1", int port=5000);
    int rc = dpi_socket_connect(host, port);
    if (rc != 0) `uvm_fatal("DPI_CONN",$sformatf("Failed to connect to agent rc=%0d",rc));
    `uvm_info("TRANS","Connected to Python agent", UVM_LOW)
  endtask

  // send batch of observations and let C parse actions; then publish items to analysis port
  function void send_obs_batch_and_publish(string json_array);
    int rc = dpi_send_obs_batch(json_array, $strlen(json_array));
    if (rc != 0) begin
      `uvm_error("TRANS","dpi_send_obs_batch failed");
      return;
    end
    rc = dpi_recv_actions_parse();
    if (rc < 0) begin
      `uvm_error("TRANS","dpi_recv_actions_parse failed");
      return;
    end

    int cnt = dpi_get_action_count();
    for (int i=0;i<cnt;i++) begin
      alu_seq_item it = alu_seq_item::type_id::create($sformatf("it_%0d", i));
      it.op = dpi_get_action_op(i);
      it.a  = dpi_get_action_a(i);
      it.b  = dpi_get_action_b(i);
      ap.write(it); // publish to driver
    end
  endfunction

  function void close();
    dpi_socket_close();
  endfunction
endclass
