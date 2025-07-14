module bram_mem # (
    parameter DATAW = 8,
    parameter DEPTH = 4,
    parameter ADDRW = $clog2(DEPTH)
)(
    input  clk,
    input  [DATAW-1:0] wdata,
    input  [ADDRW-1:0] waddr,
    input  wen,
    input  [ADDRW-1:0] raddr,
    output reg [DATAW-1:0] rdata
);

(* ramstyle = "M20K" *) reg [DATAW-1:0] mem [0:DEPTH-1];

reg [DATAW-1:0] r_wdata;
reg [ADDRW-1:0] r_waddr, r_raddr;
reg r_wen;

integer i;
initial begin
    for (i = 0; i < DEPTH; i = i + 1) begin
        mem[i] <= 0;
    end
end

always @ (posedge clk) begin
    // Register Inputs
    r_wdata <= wdata;
    r_waddr <= waddr;
    r_wen <= wen;
    r_raddr <= raddr;
    
    // Write logic
    if (r_wen) mem[r_waddr] <= r_wdata;
    // Read logic
    rdata <= mem[r_raddr];
end

endmodule