#include <iostream>
#include <iomanip>
#include <verilated.h>
#include "Vmem_cell_array.h" 

#define DATA_WIDTH 160
#define HEIGHT 128
#define CHUNK_LEN 32

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vmem_cell_array* dut = new Vmem_cell_array;

    dut->wen = 0;
    dut->raddr = 0;

    auto tick = [&]() {
        dut->clk = 1;
        dut->eval();
        dut->clk = 0; 
        dut->eval();
    };

    auto write_data = [&](int addr, int64_t data) {
        // Set up write data and address
        dut->waddr = addr;
        for (int i = 0; i < DATA_WIDTH/CHUNK_LEN; i++) {
            dut->wdata[i] = 0; // set to 0 by default
        }
        dut->wdata[0] = data & 0xFFFFFFFF;
        dut->wdata[1] = (data >> 32) & 0xFFFFFFFF; 
        dut->wen = 1;
        
        tick();
        tick();        
        dut->wen = 0;
    };

    auto read_data = [&](int addr) -> int64_t {
        dut->raddr = addr;
        tick();
        tick();
        
        // Reconstruct 64-bit value from the array
        int64_t result = dut->rdata[0];
        result |= ((int64_t)dut->rdata[1]) << 32;
        return result;
    };

    // Test initial value
    std::cout << "Initial rdata[0]: " << dut->rdata[0] << std::endl;    

    // Write test data
    int64_t test_data = 0x123456789ABCDEF0;
    std::cout << "Writing data: 0x" << std::hex << test_data << std::dec << std::endl;
    write_data(0, test_data);
    
    // Read back the data
    int64_t read_back = read_data(0);
    std::cout << "Read back data: 0x" << std::hex << read_back << std::dec << std::endl;
    
    // Test another location
    int64_t test_data2 = 0xDEADBEEFCAFEBABE;
    std::cout << "Writing data to addr 10: 0x" << std::hex << test_data2 << std::dec << std::endl;
    write_data(10, test_data2);
    
    int64_t read_back2 = read_data(10);
    std::cout << "Read back from addr 10: 0x" << std::hex << read_back2 << std::dec << std::endl;
    
    // Verify first location is still intact
    int64_t verify_first = read_data(0);
    std::cout << "Verify addr 0 still has: 0x" << std::hex << verify_first << std::dec << std::endl;

    delete dut;
    return 0;
}