// M20K BRAM function model
// Limited model, only simulates dual port access
// Missing:
// registering inputs/outputs
// Different logical sizes
// Assumes tdp mode can access full cell array

// Work on:
// logical data width/depths
// start with shallowest / widest (512 x 40), 2k x 8 and 4k x 4
// Physical width is 128 x 160
// 512 x 40 isn't supported in tdp mode, but 2kx8 and 4kx4 are

module m20k_bram_core #(
    // Logical configuration parameters
    parameter LOGICAL_DATA_WIDTH = 8,    // Logical data width (40, 8, 4)
    parameter LOGICAL_DEPTH = 2048,       // Logical depth (512, 2k, 4k)
    parameter LOGICAL_ADDR_WIDTH = log2(LOGICAL_DEPTH),

    // From: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=9786179 (CoMeFa)
    parameter PHYSICAL_ROWS = 128,
    parameter PHYSICAL_COLS = 160,
    parameter COL_MUX_FACTOR = 4, //Since widest supported width is 40 bits
    parameter PHYSICAL_ADDR_WIDTH = clog2(PHYSICAL_ROWS)
    parameter PHYSICAL_COL_WIDTH = clog2(PHYSICAL_COLS)
    // Should we have parameters for mode (ROM, SDP, TDP)?
) (
    input wire clk,
    
    // Port A
    input wire [LOGICAL_ADDR_WIDTH-1:0] addr_a,
    input wire [LOGICAL_DATA_WIDTH-1:0] data_in_a,
    input wire wen_a,
    input wire ren_a,
    output reg [LOGICAL_DATA_WIDTH-1:0] data_out_a,
    
    input wire [LOGICAL_ADDR_WIDTH-1:0] addr_b,
    input wire [LOGICAL_DATA_WIDTH-1:0] data_in_b,
    input wire wen_b,
    input wire ren_b,
    output reg [LOGICAL_DATA_WIDTH-1:0] data_out_b
);
    localparam LOG_TO_PHYS_BITS = PHYSICAL_COLS / LOGICAL_DATA_WIDTH;

    // 2D array of SRAM cells
    reg cell_array [0:PHYSICAL_ROWS-1][0:PHYSICAL_COLS-1];
    
    // Initialize physical memory
    initial begin
        integer row, col;
        for (row = 0; row < PHYSICAL_ROWS; row = row + 1) begin
            for (col = 0; col < PHYSICAL_COLS; col = col + 1) begin
                physical_memory[row][col] = 1'b0;
            end
        end
        data_out_a = {LOGICAL_DATA_WIDTH{1'b0}};
        data_out_b = {LOGICAL_DATA_WIDTH{1'b0}};
    end
    
    // For accessing words, we need to convert logical addresses to physical addresses
    function [PHYSICAL_ADDR_WIDTH:0] get_phys_row;
        input [LOGICAL_ADDR_WIDTH-1:0] logical_row;
        begin
            // Since we have many logical rows per physical row,
            // Physical row = floor(logical_row / logical_rows_per_physical_row)
            get_phys_row = logical_row/LOG_TO_PHYS_BITS;
        end
    endfunction
    
    // We need to find the starting physical column for a logical word
    function [PHYSICAL_COL_WIDTH:0] get_phys_col;
        input [LOGICAL_ADDR_WIDTH-1:0] logical_addr;
        begin
            get_phys_col = logical_addr % LOG_TO_PHYS_BITS;
        end
    endfunction
    
    // Port A access logic
    always @(posedge clk) begin
        if (write_en_a || read_en_a) begin
            // Decode logical address to physical coordinates
            reg [6:0] phys_row;
            reg [7:0] col_start;
            integer bit_idx;
            
            phys_row = get_physical_row(addr_a);
            col_start = get_col_start(addr_a);
            
            if (write_en_a) begin
                // Write logical data to physical memory
                for (bit_idx = 0; bit_idx < LOGICAL_DATA_WIDTH; bit_idx = bit_idx + 1) begin
                    if ((col_start + bit_idx) < PHYSICAL_COLS) begin
                        physical_memory[phys_row][col_start + bit_idx] <= data_in_a[bit_idx];
                    end
                end
            end
            
            if (read_en_a) begin
                // Read logical data from physical memory
                for (bit_idx = 0; bit_idx < LOGICAL_DATA_WIDTH; bit_idx = bit_idx + 1) begin
                    if ((col_start + bit_idx) < PHYSICAL_COLS) begin
                        data_out_a[bit_idx] <= physical_memory[phys_row][col_start + bit_idx];
                    end else begin
                        data_out_a[bit_idx] <= 1'b0; // Padding for out-of-bounds
                    end
                end
            end
        end
    end
    
    // Port B access logic (similar to Port A)
    always @(posedge clk) begin
        if (write_en_b || read_en_b) begin
            reg [6:0] phys_row;
            reg [7:0] col_start;
            integer bit_idx;
            
            phys_row = get_physical_row(addr_b);
            col_start = get_col_start(addr_b);
            
            if (write_en_b) begin
                for (bit_idx = 0; bit_idx < LOGICAL_DATA_WIDTH; bit_idx = bit_idx + 1) begin
                    if ((col_start + bit_idx) < PHYSICAL_COLS) begin
                        physical_memory[phys_row][col_start + bit_idx] <= data_in_b[bit_idx];
                    end
                end
            end
            
            if (read_en_b) begin
                for (bit_idx = 0; bit_idx < LOGICAL_DATA_WIDTH; bit_idx = bit_idx + 1) begin
                    if ((col_start + bit_idx) < PHYSICAL_COLS) begin
                        data_out_b[bit_idx] <= physical_memory[phys_row][col_start + bit_idx];
                    end else begin
                        data_out_b[bit_idx] <= 1'b0;
                    end
                end
            end
        end
    end
    
    // Debug: Display physical memory usage
    `ifdef DEBUG_PHYSICAL
    always @(posedge clk) begin
        if (write_en_a) begin
            $display("Physical Write Port A: Row=%d, Col_Start=%d, Width=%d, Data=%h", 
                     get_physical_row(addr_a), get_col_start(addr_a), LOGICAL_DATA_WIDTH, data_in_a);
        end
        if (write_en_b) begin
            $display("Physical Write Port B: Row=%d, Col_Start=%d, Width=%d, Data=%h", 
                     get_physical_row(addr_b), get_col_start(addr_b), LOGICAL_DATA_WIDTH, data_in_b);
        end
    end
    `endif

endmodule