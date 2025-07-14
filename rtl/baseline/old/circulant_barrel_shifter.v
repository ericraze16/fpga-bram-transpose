// Enhanced circulant buffer with integrated barrel shifter for clean transpose output
// Builds on single_word_circulant by adding output barrel shifting

module circulant_barrel_shifter #(
    parameter MATRIX_DIM = 4, //Assume square
    parameter COL_WIDTH = 8,
    parameter WORD_LEN = 32,
    parameter ADDR_LEN = $clog2(MATRIX_DIM)
)(
    input wire clk,

    // Write interface
    input wire [WORD_LEN-1:0] data_in,
    input wire write_en,
    input wire [ADDR_LEN-1:0] write_row,
    input wire [ADDR_LEN-1:0] write_col,

    // Read interface
    input wire read_en,
    input wire [ADDR_LEN-1:0] read_row,
    input wire [ADDR_LEN-1:0] read_col,
    
    // Output options
    input wire barrel_shift_en, // Enable barrel shifting for clean transpose
    output reg [WORD_LEN-1:0] data_out
);

// TODO: add checking for valid sizes
localparam COLS_PER_WORD = WORD_LEN / COL_WIDTH;
localparam ADDR_MASK = MATRIX_DIM - 1; // Restricts matrix_dim to a power of 2!!
localparam SHIFT_BITS = $clog2(COLS_PER_WORD);

// Memory array (compiled into BRAMs)
reg [COL_WIDTH-1:0] col_mem [0:MATRIX_DIM-1][0:MATRIX_DIM-1];

// Pipeline registers for barrel shifter
reg [WORD_LEN-1:0] raw_data_out;
reg [SHIFT_BITS-1:0] shift_amount;
reg barrel_shift_en_reg;

// Circulant address calculation w/ fixed bit offsets
function [ADDR_LEN-1:0] circulant_col_addr;
    input [ADDR_LEN-1:0] row;
    input [ADDR_LEN-1:0] base_col; // word start
    input [ADDR_LEN-1:0] col_offset; // chunk start
    begin
        // Shift each row right by 1 more column, handle wrap-around
        circulant_col_addr = (row + base_col + col_offset) & ADDR_MASK;
    end
endfunction

// Barrel shifter function - shifts right by shift_amount chunks
function [WORD_LEN-1:0] barrel_shift_right;
    input [WORD_LEN-1:0] barrel_in;
    input [SHIFT_BITS-1:0] shift_amt;
    reg [WORD_LEN-1:0] shifted_data;
    integer chunk_idx;
    reg [SHIFT_BITS-1:0] src_chunk;
    begin
        shifted_data = {WORD_LEN{1'b0}};
        for (chunk_idx = 0; chunk_idx < COLS_PER_WORD; chunk_idx = chunk_idx + 1) begin
            // Calculate source chunk index with wrap-around
            src_chunk = (chunk_idx[SHIFT_BITS-1:0] + shift_amt) & (COLS_PER_WORD - 1);
            // Move the chunk to its new position
            shifted_data[chunk_idx * COL_WIDTH +: COL_WIDTH] = 
                barrel_in[src_chunk * COL_WIDTH +: COL_WIDTH];
        end
        barrel_shift_right = shifted_data;
    end
endfunction

// Write logic
integer w_chunk;
reg [ADDR_LEN-1:0] s_w_col; // shifted write col
always @(posedge clk) begin
    if (write_en) begin
        // Loop over data_in's chunks, store each in the right column
        for (w_chunk = 0; w_chunk < COLS_PER_WORD; w_chunk = w_chunk + 1) begin
            s_w_col = circulant_col_addr(write_row, write_col, w_chunk[ADDR_LEN-1:0]);
            col_mem[write_row][s_w_col] <= data_in[w_chunk * COL_WIDTH +: COL_WIDTH];
        end
    end
end

// Read logic with pipeline stage
integer r_chunk;
reg [ADDR_LEN-1:0] s_r_col; // shifted read col
reg [ADDR_LEN-1:0] current_r_row; // increment read row each chunk
always @(posedge clk) begin
    if (read_en) begin
        current_r_row = read_row; // reset row for each read
        for (r_chunk = 0; r_chunk < COLS_PER_WORD; r_chunk = r_chunk + 1) begin
            s_r_col = circulant_col_addr(read_row, read_col, r_chunk[ADDR_LEN-1:0]);
            raw_data_out[r_chunk * COL_WIDTH +: COL_WIDTH] <= col_mem[current_r_row][s_r_col];
            current_r_row = (current_r_row + 1'b1) & ADDR_MASK; // increment row for next chunk
        end
        
        // Calculate shift amount for barrel shifter
        // For clean transpose: shift_amount = (COLS_PER_WORD - read_row) % COLS_PER_WORD
        shift_amount <= (COLS_PER_WORD[SHIFT_BITS-1:0] - read_row[SHIFT_BITS-1:0]) & (COLS_PER_WORD[SHIFT_BITS-1:0] - 1'b1);
        barrel_shift_en_reg <= barrel_shift_en;
    end
end

// Barrel shifter output stage
always @(posedge clk) begin
    if (barrel_shift_en_reg) begin
        data_out <= barrel_shift_right(raw_data_out, shift_amount);
    end else begin
        data_out <= raw_data_out; // Raw circulant output
    end
end

endmodule