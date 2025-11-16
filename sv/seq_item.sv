// sv/seq_item.sv
`include "uvm_pkg.sv"
class alu_seq_item extends uvm_sequence_item;
  rand bit [7:0] a;
  rand bit [7:0] b;
  rand bit [2:0] op;

  `uvm_object_utils(alu_seq_item)
  function new(string name="alu_seq_item");
    super.new(name);
  endfunction
endclass
