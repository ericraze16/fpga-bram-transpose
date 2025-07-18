
module circulant_barrel_shifter_v2 #(
    parameter MATRIX_DIM = 4, //Assume square
    parameter MEM_WIDTH = 8,
    parameter ROW_WIDTH = MATRIX_DIM * MEM_WIDTH, 
    parameter ADDR_LEN = $clog2(MATRIX_DIM)
)(
    input wire clk,

    // Write interface - we have memory column in our abstracted 2-d array
    input wire [ROW_WIDTH-1:0] wdata,
    input wire wen,
    input wire [ADDR_LEN-1:0] waddr, // base row addr 

    // Read interface
    input wire ren,
    input wire [ADDR_LEN-1:0] rTransAddr, // base row addr
    
    output reg [ROW_WIDTH-1:0] rTransData
);

// Registers for clocking the input signals
reg [ROW_WIDTH-1:0] r_wdata;
reg [ADDR_LEN-1:0] r_waddr, r_rTransAddr;
reg r_wen, r_ren;

// Wires to interface with BRAM modules
reg [MEM_WIDTH-1:0] bram_wdata [0:MATRIX_DIM-1]; 
reg [ADDR_LEN-1:0] bram_waddr [0:MATRIX_DIM-1];
reg bram_wen [0:MATRIX_DIM-1];
reg [ADDR_LEN-1:0] bram_raddr [0:MATRIX_DIM-1];
wire [MEM_WIDTH-1:0] bram_rdata [0:MATRIX_DIM-1];

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
    input [ADDR_LEN-1:0] addr, //ie starting row
    input [ADDR_LEN-1:0] chunk_idx);
    begin
        reg [ADDR_LEN:0] temp_addr; // temp_addr needs to be wide enough to handle overflow
        // specifically, it needs to store 2 * matrix_dim in the worst case, so add an extra bit
        temp_addr = addr + chunk_idx;
        circ_col_addr = (temp_addr >= MATRIX_DIM) ? 
                        (temp_addr - (MATRIX_DIM)) : temp_addr;
    end
endfunction

// Handle writes - register input signals for write operations
// We want to register the input signals to the circulant shift calculation for timing
always @(posedge clk) begin
    r_wdata <= wdata;
    r_waddr <= waddr;
    r_wen <= wen;
end

integer w_chunk_idx, j;
always @(*) begin
    reg [ADDR_LEN-1:0] circ_wmem; // Handles circulant mem addressing

    // Using the registered values, distribute the writes to the BRAMs
    if (r_wen) begin
        for (w_chunk_idx = 0; w_chunk_idx < MATRIX_DIM; w_chunk_idx = w_chunk_idx + 1) begin
            circ_wmem = circ_col_addr(r_waddr, w_chunk_idx);
            bram_waddr[circ_wmem] = r_waddr;
            bram_wdata[circ_wmem] = r_wdata[(w_chunk_idx * MEM_WIDTH) +: MEM_WIDTH];
            bram_wen[circ_wmem] = 1'b1;
        end
    end else begin
        for (j = 0; j < MATRIX_DIM; j = j + 1) begin
            bram_wen[j] = 1'b0; // Disable write enables if not writing
        end
    end
end

// Handle distributing reads to BRAMs
// First, we want to set the read address:
integer rchunk_idx;
reg [ADDR_LEN-1:0] circ_rmem; // Handles circulant mem addressing

always @(posedge clk) begin
    r_rTransAddr <= rTransAddr;
    r_ren <= ren;
    // Need to handle the start of the data being in an offset 
    // We need to read in a diagonal pattern starting at row=0, column=r_rTransAddr 
    if (r_ren) begin
        for (rchunk_idx = 0; rchunk_idx < MATRIX_DIM; rchunk_idx = rchunk_idx + 1) begin
            circ_rmem = circ_col_addr(r_rTransAddr, rchunk_idx);
            bram_raddr[circ_rmem] <= rchunk_idx[ADDR_LEN-1:0];
        end
    end else begin
        for (rchunk_idx = 0; rchunk_idx < MATRIX_DIM; rchunk_idx = rchunk_idx + 1) begin
            bram_raddr[rchunk_idx] <= 0;
        end
    end
end

// Handle collecting read data from mems
reg [ADDR_LEN-1:0] circ_rCollectMem;
always @(posedge clk) begin
    // Need to rotate left by r_rTransAddr
    reg [ADDR_LEN-1:0] circ_rCollectMem; // Handles circulant mem addressing
    for (rchunk_idx = 0; rchunk_idx < MATRIX_DIM; rchunk_idx = rchunk_idx + 1) begin
        circ_rCollectMem = circ_col_addr(r_rTransAddr, rchunk_idx);
        rTransData[(rchunk_idx * MEM_WIDTH) +: MEM_WIDTH] <= bram_rdata[circ_rCollectMem];
    end
end

endmodule