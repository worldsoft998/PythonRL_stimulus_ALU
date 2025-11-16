// rtl/alu_dut.sv
module alu_dut (
  input  logic clk, input logic rst_n,
  input  logic [7:0] a, input logic [7:0] b,
  input  logic [2:0] op, input logic start,
  output logic done, output logic [7:0] result,
  output logic err
);

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      done <= 0; result <= 0; err <= 0;
    end else begin
      if (start) begin
        unique case (op)
          3'd0: result <= a + b;
          3'd1: result <= a - b;
          3'd2: result <= a & b;
          3'd3: result <= a | b;
          3'd4: result <= a ^ b;
          default: begin result <= 8'h00; err <= 1; end
        endcase
        done <= 1;
      end else begin
        done <= 0; err <= 0;
      end
    end
  end

endmodule
