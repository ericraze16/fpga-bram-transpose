// M20K BRAM functional model
// Supports dual port true dual-port mode
// Supports set of logical widths
// Physical organization: 128 rows × 160 columns (individual bit cells)

// What happens when:
// write to the same address over both ports?

module m20k_bram_core #(
    // Logical configuration parameters
    parameter LOGICAL_DATA_WIDTH = 8,     // Logical data width (40, 8, 4)
    parameter LOGICAL_DEPTH = 2048,       // Logical depth (512, 2k, 4K)
    
    // Physical parameters from https://ieeexplore.ieee.org/document/9786179
    parameter PHYSICAL_ROWS = 128,
    parameter PHYSICAL_COLS = 160,
    parameter COL_MUX_FACTOR = 4          // Since widest supported width is 40 bits
) (
    input wire clk,
    input wire rst,
    
    // Port A
    input wire [$clog2(LOGICAL_DEPTH)-1:0] addr_a,
    input wire [LOGICAL_DATA_WIDTH-1:0] data_in_a,
    input wire wen_a,
    input wire ren_a,
    output reg [LOGICAL_DATA_WIDTH-1:0] data_out_a,
    
    // Port B
    input wire [$clog2(LOGICAL_DEPTH)-1:0] addr_b,
    input wire [LOGICAL_DATA_WIDTH-1:0] data_in_b,
    input wire wen_b,
    input wire ren_b,
    output reg [LOGICAL_DATA_WIDTH-1:0] data_out_b
);

    // Local parameters
    localparam ADDR_WIDTH = $clog2(LOGICAL_DEPTH);
    localparam PHYSICAL_ADDR_WIDTH = $clog2(PHYSICAL_ROWS);
    localparam PHYSICAL_COL_WIDTH = $clog2(PHYSICAL_COLS);
    localparam LOG_TO_PHYS_BITS = PHYSICAL_COLS / LOGICAL_DATA_WIDTH;

    // 2D array of individual SRAM cells 
    reg cell_array [0:PHYSICAL_ROWS-1][0:PHYSICAL_COLS-1];
    
    // Variables for address decoding 
    reg [PHYSICAL_ADDR_WIDTH-1:0] phys_row_a, phys_row_b;
    reg [PHYSICAL_COL_WIDTH-1:0] col_start_a, col_start_b;
    integer bit_idx_a, bit_idx_b; // Loop variables

    // Registerd inputs
    // Port A
    reg [$clog2(LOGICAL_DEPTH)-1:0] r_addr_a;
    reg [LOGICAL_DATA_WIDTH-1:0] r_data_in_a;
    reg r_wen_a;
    reg r_ren_a;
    // Port B
    reg [$clog2(LOGICAL_DEPTH)-1:0] r_addr_b;
    reg [LOGICAL_DATA_WIDTH-1:0] r_data_in_b;
    reg r_wen_b;
    reg r_ren_b;
    
    // Initialize physical memory
    initial begin
        integer row, col;
        for (row = 0; row < PHYSICAL_ROWS; row = row + 1) begin
            for (col = 0; col < PHYSICAL_COLS; col = col + 1) begin
                cell_array[row][col] = 1'b0;
            end
        end
        data_out_a = {LOGICAL_DATA_WIDTH{1'b0}};
        data_out_b = {LOGICAL_DATA_WIDTH{1'b0}};
    end
    
    // Reset logic
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            // Reset outputs and registered inputs
            data_out_a <= {LOGICAL_DATA_WIDTH{1'b0}};
            data_out_b <= {LOGICAL_DATA_WIDTH{1'b0}};

            r_addr_a <= {ADDR_WIDTH{1'b0}};
            r_data_in_a <= {LOGICAL_DATA_WIDTH{1'b0}};
            r_wen_a <= 1'b0;
            r_ren_a <= 1'b0;

            r_addr_b <= {ADDR_WIDTH{1'b0}};
            r_data_in_b <= {LOGICAL_DATA_WIDTH{1'b0}};
            r_wen_b <= 1'b0;
            r_ren_b <= 1'b0;
        end
        else begin
            // Register inputs
            r_addr_a <= addr_a;
            r_data_in_a <= data_in_a;
            r_wen_a <= wen_a;
            r_ren_a <= ren_a;

            r_addr_b <= addr_b;
            r_data_in_b <= data_in_b;
            r_wen_b <= wen_b;
            r_ren_b <= ren_b;
        end
    end
    
    // Address mapping functions - your original logic, corrected
    function [PHYSICAL_ADDR_WIDTH-1:0] get_phys_row;
        input [ADDR_WIDTH-1:0] logical_addr;
        begin
            // Physical row = floor(logical_addr / logical_words_per_physical_row)
            get_phys_row = logical_addr / LOG_TO_PHYS_BITS;
        end
    endfunction
    
    // Find the starting physical column for a logical word
    function [PHYSICAL_COL_WIDTH-1:0] get_phys_col;
        input [ADDR_WIDTH-1:0] logical_addr;
        begin
            // Column start = (logical_addr % words_per_row) * logical_data_width
            get_phys_col = (logical_addr % LOG_TO_PHYS_BITS) * LOGICAL_DATA_WIDTH;
        end
    endfunction
    
    // Port A access logic (your original approach, fixed)
    always @(posedge clk) begin
        if (!rst) begin
            if (r_wen_a | r_ren_a) begin
                // Decode logical address to physical coordinates
                phys_row_a = get_phys_row(r_addr_a);
                col_start_a = get_phys_col(r_addr_a);
                
                if (r_wen_a) begin
                    // Write logical data to individual physical cells
                    for (bit_idx_a = 0; bit_idx_a < LOGICAL_DATA_WIDTH; bit_idx_a = bit_idx_a + 1) begin
                        if ((col_start_a + bit_idx_a) < PHYSICAL_COLS) begin
                            cell_array[phys_row_a][col_start_a + bit_idx_a] <= r_data_in_a[bit_idx_a];
                        end
                    end
                end
                
                if (r_ren_a) begin
                    // Read logical data from individual physical cells
                    for (bit_idx_a = 0; bit_idx_a < LOGICAL_DATA_WIDTH; bit_idx_a = bit_idx_a + 1) begin
                        if ((col_start_a + bit_idx_a) < PHYSICAL_COLS) begin
                            data_out_a[bit_idx_a] <= cell_array[phys_row_a][col_start_a + bit_idx_a];
                        end else begin
                            data_out_a[bit_idx_a] <= 1'b0; // Default to 0 for out-of-bounds
                        end
                    end
                end
            end
        end
    end
    
    // Port B access logic (identical to Port A because of naive dual-port)
    always @(posedge clk) begin
        if (!rst) begin
            if (r_wen_b | r_ren_b) begin
                phys_row_b = get_phys_row(r_addr_b);
                col_start_b = get_phys_col(r_addr_b);
                
                if (r_wen_b) begin
                    // Write logical data to individual physical cells
                    for (bit_idx_b = 0; bit_idx_b < LOGICAL_DATA_WIDTH; bit_idx_b = bit_idx_b + 1) begin
                        if ((col_start_b + bit_idx_b) < PHYSICAL_COLS) begin
                            cell_array[phys_row_b][col_start_b + bit_idx_b] <= data_in_b[bit_idx_b];
                        end
                    end
                end
                
                if (r_ren_b) begin
                    // Read logical data from individual physical cells
                    for (bit_idx_b = 0; bit_idx_b < LOGICAL_DATA_WIDTH; bit_idx_b = bit_idx_b + 1) begin
                        if ((col_start_b + bit_idx_b) < PHYSICAL_COLS) begin
                            data_out_b[bit_idx_b] <= cell_array[phys_row_b][col_start_b + bit_idx_b];
                        end else begin
                            data_out_b[bit_idx_b] <= 1'b0;
                        end
                    end
                end
            end
        end
    end
    
    // Conservative assumption: for now, assume diff logical address on same physical row is a collision
    reg collision = (r_wen_a | r_ren_a) && (r_wen_b | r_ren_b) && 
                     (get_phys_row(r_addr_a) == get_phys_row(r_addr_b)) &&
                     (r_wen_a | r_wen_b);  // At least one write
    
    // Debug: print registered inputs and collision status
    `ifdef DEBUG_M20K
    always @(posedge clk) begin
        if (collision) begin
            $display("WARNING: Memory collision detected at physical row %d (addr_a=%d, addr_b=%d)", 
                     get_phys_row(r_addr_a), r_addr_a, r_addr_b);
        end
        if (r_wen_a) begin
            $display("Physical Write Port A: Row=%d, Col_Start=%d, Width=%d, Data=%h", 
                     get_phys_row(r_addr_a), get_phys_col(r_addr_a), LOGICAL_DATA_WIDTH, r_data_in_a);
        end
        if (r_wen_b) begin
            $display("Physical Write Port B: Row=%d, Col_Start=%d, Width=%d, Data=%h", 
                     get_phys_row(r_addr_b), get_phys_col(r_addr_b), LOGICAL_DATA_WIDTH, r_data_in_b);
        end
    end
    `endif

    // Assertions for verification - dont use registered inputs, want instant feedback on inputs
    `ifdef FORMAL
    always @(posedge clk) begin
        // Check that logical addresses are within bounds
        if (wen_a || ren_a) 
            assert(addr_a < LOGICAL_DEPTH) else $error("Port A address out of bounds");
        if (wen_b || ren_b) 
            assert(addr_b < LOGICAL_DEPTH) else $error("Port B address out of bounds");
            
        // Check that physical addresses are within bounds
        if (wen_a || ren_a)
            assert(get_phys_row(addr_a) < PHYSICAL_ROWS) else $error("Physical row A out of bounds");
        if (wen_b || ren_b)
            assert(get_phys_row(addr_b) < PHYSICAL_ROWS) else $error("Physical row B out of bounds");
            
        // Check that column access is within bounds
        if (wen_a || ren_a)
            assert((get_phys_col(addr_a) + LOGICAL_DATA_WIDTH) <= PHYSICAL_COLS) 
                else $error("Physical column A access out of bounds");
        if (wen_b || ren_b)
            assert((get_phys_col(addr_b) + LOGICAL_DATA_WIDTH) <= PHYSICAL_COLS) 
                else $error("Physical column B access out of bounds");
    end
    `endif

endmodule