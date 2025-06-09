// Basic read transposed matrix
// Inspired by https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=8892195&tag=1
// 4x4 matrix stored in a circulant pattern, where each column of the ciculant
// matrix is stored in its own BRAM

module basic_cicrculant (
    input wire clk,

    // Assume 1 element per cycle write/read
    input wire [7:0] data_in,
    input wire write_en,
    input wire [1:0] write_row,
    input wire [1:0] write_col,

    input wire read_en,
    input wire [1:0] read_row,
    input wire [1:0] read_col,
    output wire [7:0] data_out
);

// Define memories (compiled into BRAMs)
reg [7:0] col0_mem [3:0];
reg [7:0] col1_mem [3:0];
reg [7:0] col2_mem [3:0];
reg [7:0] col3_mem [3:0];

// Ciculant address calculation
function [1:0] circulant_col_addr;
    input [1:0] row;
    input [1:0] col;
    begin
        // Each element needs to be shifted by 1 column more than the previous row
        circulant_col_addr = (col + row) & 2'b11;
    end
endfunction

// Write logic
always @(posedge clk) begin
    if (write_en) begin
        reg [1:0] circ_write_col;
        circ_write_col = circulant_col_addr(write_row, write_col);
        case (circ_write_col)
            2'b00: col0_mem[write_row] <= data_in;
            2'b01: col1_mem[write_row] <= data_in;
            2'b10: col2_mem[write_row] <= data_in;
            2'b11: col3_mem[write_row] <= data_in;
            // Add other cases as needed
        endcase 
    end
end

// Read transposed logic
reg [7:0] read_data;
always @(*) begin
    if (write_en) begin
        reg [1:0] circ_read_col;
        circ_read_col = circulant_col_addr(read_row, read_col);
        case (circ_read_col)
            2'b00: read_data = col0_mem[read_row];
            2'b01: read_data = col1_mem[read_row];
            2'b10: read_data = col2_mem[read_row];
            2'b11: read_data = col3_mem[read_row];
            default: read_data = 8'b0;
        endcase
    end
end
assign data_out = read_data;

endmodule


