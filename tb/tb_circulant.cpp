// Verilator C++ testbench
// Save as: tb_circulant.cpp

#include <iostream>
#include <verilated.h>
#include "Vbasic_circulant.h" 

using namespace std;

int main(int argc, char** argv) {
    // Initialize Verilator
    Verilated::commandArgs(argc, argv);
    
    // Create instance of our module
    Vbasic_circulant* dut = new Vbasic_circulant;
    
    // Initialize signals
    dut->clk = 0;
    dut->write_en = 0;
    dut->read_en = 0;
    
    cout << "=== CIRCULANT MATRIX TEST ===" << endl;
    
    // Helper function to toggle clock
    auto tick = [&]() {
        dut->clk = 1;
        dut->eval();
        dut->clk = 0; 
        dut->eval();
    };
    
    // 1. Write the matrix (will be stored in a circulant pattern)
    cout << "\n1. Writing 4x4 Matrix:" << endl;
    cout << "   [11, 12, 13, 14]" << endl;
    cout << "   [21, 22, 23, 24]" << endl;
    cout << "   [31, 32, 33, 34]" << endl;
    cout << "   [41, 42, 43, 44]" << endl;
    
    dut->write_en = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            dut->write_row = i;
            dut->write_col = j;
            dut->data_in = 10*(i+1) + (j+1);  // Creates 11,12,13,14,21,22...
            tick();
        }
    }
    dut->write_en = 0;
    
    // 2. Read back original matrix (should be transposed)
    cout << "\n2. Read in transpose direction:" << endl;
    dut->read_en = 1;
    
    for (int i = 0; i < 4; i++) {
        cout << "   Row " << i << ": [";
        for (int j = 0; j < 4; j++) {
            dut->read_row = j;
            dut->read_col = i;
            dut->eval();  // Evaluate combinational logic
            
            cout << (int)dut->data_out;
            if (j < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    
    dut->read_en = 0;
    
    cout << "\n=== TEST COMPLETE ===" << endl;

    delete dut;
    return 0;
}