
module circulant_barrel_shifter_v2 #(
    parameter MATRIX_DIM = 4, //Assume square
    parameter MEM_WIDTH = 8,
    parameter ROW_WIDTH = MATRIX_DIM * MEM_WIDTH, 
    parameter ADDR_LEN = $clog2(MATRIX_DIM)
)(
    input wire clk,

    // Write interface - we have memory column in our abstracted 2-d array
    input wire [ROW_WIDTH-1:0] data_in,
    input wire wen,
    input wire [ADDR_LEN-1:0] w_base_mem, // base mem column
    input wire [ADDR_LEN-1:0] w_base_addr, // base row addr 

    // Read interface
    input wire ren,
    input wire [ADDR_LEN-1:0] r_base_mem, // base mem column
    input wire [ADDR_LEN-1:0] r_base_addr, // base row addr
    input wire [ROW_WIDTH-1:0] data_out,
    
    // Output options
    input wire barrel_shift_en, // Enable barrel shifting for clean transpose
    output reg [ROW_WIDTH-1:0] data_out
);

// Wires to interface with BRAM modules
wire [MEM_WIDTH-1:0] bram_wdata [0:MATRIX_DIM-1]; 
wire [ADDR_LEN-1:0] bram_waddr [0:MATRIX_DIM-1];
wire bram_wen [0:MATRIX_DIM-1];
wire [ADDR_LEN-1:0] bram_raddr [0:MATRIX_DIM-1];
wire [COL_WIDTH-1:0] bram_rdata [0:MATRIX_DIM-1];

// Registers for pipelined barrel shifter
reg [ROW_WIDTH-1:0] raw_data_out;
reg [ADDR_LEN-1:0] shift_amount;

// Generate BRAM instances
genvar mem_idx;
generate
    for (mem_idx = 0; mem_idx < MATRIX_DIM; mem_idx = mem_idx + 1) begin : bram_gen 
        // Each column is a separate BRAM instance
        bram_mem #(
            .DATAW(MEM_WIDTH),
            .DEPTH(MATRIX_DIM),
            .ADDRW(ADDR_LEN)
        ) bram_inst (
            .clk(clk),
            .wdata(bram_wdata[mem_idx]),
            .waddr(bram_waddr[mem_idx]),
            .wen(bram_wen[mem_idx]),
            .raddr(bram_raddr[mem_idx]),
            .rdata(bram_rdata[mem_idx])
        );
    end
endgenerate

// Generate circulant col addr
function [ADDR_LEN-1:0] circ_col_addr(
    input [ADDR_LEN-1:0] mem,
    input [ADDR_LEN-1:0] addr,
    input [ADDR_LEN-1:0] chunk_idx);
    begin
        cir_col_addr = (mem + addr + chunk_idx) & (MATRIX_DIM - 1); // Handle wrap-around
    end
endfunction

// TODO: Barrel shift logic


// Handle writes
integer w_chunk_idx;
wire [ADDR_LEN-1:0] circ_w_mem;
always @(*) begin
    if (wen) begin
        for(w_chunk_idx = 0; w_chunk_idx < MATRIX_DIM; w_chunk_idx = w_chunk_idx + 1) begin
            circ_w_mem = circ_col_addr(w_base_mem, w_base_addr, w_chunk_idx); // Acceess mem cols in circ pattern
            bram_waddr[circ_w_mem] = w_base_addr;
            bram_wdata[circ_w_mem] = data_in[(w_chunk_idx * MEM_WIDTH) +: MEM_WIDTH];
            bram_wen[circ_w_mem] = 1'b1;
        end
    end
end

// Handle distributing reads to BRAMs
integer r_chunk_idx;
wire [ADDR_LEN-1:0] circ_r_mem;
wire [ADDR_LEN-1:0] incr_r_addr;
always @(*) begin
    if (ren) begin
        incr_r_addr = r_base_addr;
        for(r_chunk_idx = 0; r_chunk_idx < MATRIX_DIM; r_chunk_idx = r_chunk_idx + 1) begin
            incr_r_addr = (r_base_addr + r_chunk_idx) & (MATRIX_DIM - 1); // Increment row for diagonal reads
            circ_r_mem = circ_col_addr(r_base_mem, r_base_addr, r_chunk_idx); // Access mem cols in circ pattern - USE BASE ROW HERE
            bram_raddr[circ_r_mem] = incr_r_addr;
        end
        shift_amount <= (MATRIX_DIM - r_base_addr) & (MATRIX_DIM - 1); // I think this should be clocked?
    end
end




endmodule