// M20K BRAM function model
// Limited model, only simulates dual port access
// Missing:
// registering inputs/outputs
// Different logical sizes
// Assumes tdp mode can access full cell array

module m20k_bram_core #(
    parameter DATA_WIDTH = 160, // 160 physical columns
    parameter ADDR_WIDTH = 7,        // 128 locations for 160-bit mode
    parameter MEMORY_SIZE = 20480     // 20K bit
) (
    input wire clk,
    
    // Port A
    input wire [ADDR_WIDTH-1:0] addr_a,
    input wire [DATA_WIDTH-1:0] data_in_a,
    input wire write_en_a,
    input wire read_en_a,
    output reg [DATA_WIDTH-1:0] data_out_a,
    
    // Port B
    input wire [ADDR_WIDTH-1:0] addr_b,
    input wire [DATA_WIDTH-1:0] data_in_b,
    input wire write_en_b,
    input wire read_en_b,
    output reg [DATA_WIDTH-1:0] data_out_b
);

    // Memory cell array
    localparam MEM_DEPTH = (1 << ADDR_WIDTH);
    reg cell_array [0:MEM_DEPTH-1][0:DATA_WIDTH-1];
    
    // Initialize memory to zero
    initial begin
        integer i;
        for (i = 0; i < MEM_DEPTH; i = i + 1) begin
            memory[i] = {DATA_WIDTH{1'b0}};
        end
        data_out_a = {DATA_WIDTH{1'b0}};
        data_out_b = {DATA_WIDTH{1'b0}};
    end
    
    // Port A access logic - assume write before read
    always @(posedge clk) begin

        if (write_en_a) begin
            memory[addr_a] <= data_in_a;
        end
        

        if (read_en_a) begin
            data_out_a <= memory[addr_a];
        end
    end
    
    // Port B access logic - assume write before read
    always @(posedge clk) begin

        if (write_en_b) begin
            memory[addr_b] <= data_in_b;
        end
        
        if (read_en_b) begin
            data_out_b <= memory[addr_b];
        end
    end

endmodule