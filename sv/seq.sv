// sv/seq.sv
`include "uvm_pkg.sv"
class alu_test_seq extends uvm_sequence #(alu_seq_item);

  `uvm_object_utils(alu_test_seq)
  function new(string name="alu_test_seq"); super.new(name); endfunction

  task body();
    alu_seq_item item;
    repeat (5) begin
      item = alu_seq_item::type_id::create("item");
      item.randomize();
      start_item(item);
      finish_item(item);
    end
  endtask
endclass
