module mem_cell_array #(
    parameter PHYS_COLS = 160,
    parameter PHYS_ROWS = 128, 
    // 20k bit BRAM
    parameter DATA_WIDTH = PHYS_COLS,
    parameter HEIGHT = PHYS_ROWS,
    parameter ADDR_LEN = $clog2(HEIGHT)

    // How to model the partial word lines - this should be an input to the module, not a parameter
)(
    input wire clk,
    input wire [DATA_WIDTH-1:0] wdata,
    input wire [ADDR_LEN-1:0] waddr, 
    input wire wen,
    input wire [ADDR_LEN-1:0] raddr,
    output reg [DATA_WIDTH-1:0] rdata
);

// Internal cell array - we will build on this for partial word lines
reg [DATA_WIDTH-1:0] cell_array [0:HEIGHT-1];

// Clocked input signals
reg [ADDR_LEN-1:0] r_raddr, r_waddr;
reg [DATA_WIDTH-1:0] r_wdata;
reg r_wen;

// Initialize
integer i;
initial begin
    for (i =0; i < HEIGHT; i = i + 1) begin
        cell_array[i] = 0;
    end
end

always @(posedge clk) begin

// clock input signals
    r_wen <= wen;
    r_wdata <= wdata;
    r_waddr <= waddr;
    r_raddr <= raddr;

    if (r_wen) begin
        cell_array[r_waddr] <= r_wdata;
    end 
    rdata <= cell_array[r_raddr];

end


endmodule