// Builds on basic_circulant by allowing for wide data word (multi column width)
// Parametric word, matrix and column sizes
// Circulant shifting accounts for parametric sizes

module single_word_circulant #(
    parameter MATRIX_DIM = 4, //Assume square
    parameter COL_WIDTH = 8,
    parameter WORD_LEN = 32,
    parameter ADDR_LEN = $clog2(MATRIX_DIM) 
)(
    input wire clk,

    // Assume 1 element per cycle write/read
    input wire [WORD_LEN-1:0] data_in,
    input wire write_en,
    input wire [ADDR_LEN-1:0] write_row,
    input wire [ADDR_LEN-1:0] write_col,

    input wire read_en,
    input wire [ADDR_LEN-1:0] read_row,
    input wire [ADDR_LEN-1:0] read_col,
    output reg [WORD_LEN-1:0] data_out
);

// TODO: add checking for valid sizes
localparam COLS_PER_WORD = WORD_LEN / COL_WIDTH;
localparam ADDR_MASK = MATRIX_DIM - 1; // Restricts col_width to a power of 2!!

// Memory array (compiled into BRAMs)
reg [COL_WIDTH-1:0] col_mem [0:MATRIX_DIM-1][0:MATRIX_DIM-1];

// Ciculant address calculation w/ fixed bit offsets
function [ADDR_LEN-1:0] circulant_col_addr;
    input [ADDR_LEN-1:0] row;
    input [ADDR_LEN-1:0] base_col; // word start
    input [ADDR_LEN-1:0] col_offset; // chunk start
    begin
        // Shift each row right by 1 more column, handle wrap-around
        circulant_col_addr = (row + base_col + col_offset) & ADDR_LEN'(ADDR_MASK);
    end
endfunction

// Write logic
integer w_chunk;
reg [ADDR_LEN-1:0] s_w_col; // shifted write col - Fixed width
always @(posedge clk) begin
    if (write_en) begin
        // Loop over data_in's chunks, store each in the right column
        for (w_chunk = 0; w_chunk < COLS_PER_WORD; w_chunk = w_chunk + 1) begin
            s_w_col <= circulant_col_addr(write_row, write_col, w_chunk[ADDR_LEN-1:0]);
            col_mem[write_row][s_w_col] <= data_in[w_chunk * COL_WIDTH +: COL_WIDTH];
        end
    end
end

// Read logic
integer r_chunk;
reg [ADDR_LEN-1:0] s_r_col; // shifted read col
reg [ADDR_LEN-1:0] r_row; // increment read row each chunk
always @(posedge clk) begin
    if (read_en) begin
        r_row <= read_row; // reset row for each read
        for (r_chunk = 0; r_chunk < COLS_PER_WORD; r_chunk = r_chunk + 1) begin
            s_r_col <= circulant_col_addr(read_row, read_col, r_chunk[ADDR_LEN-1:0]);
            data_out[r_chunk * COL_WIDTH +: COL_WIDTH] <= col_mem[r_row][s_r_col];
            r_row <= (r_row + 1'b1) & ADDR_LEN'(ADDR_MASK); // increment row for next chunk
        end
    end
end

endmodule