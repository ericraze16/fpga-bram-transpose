// Verilator C++ testbench
// Save as: tb_circulant.cpp

#include <iostream>
#include <verilated.h>
#include "Vbasic_circulant.h" 

using namespace std;

int main(int argc, char** argv) {
    // Initialize
    Verilated::commandArgs(argc, argv);
    Vbasic_circulant* dut = new Vbasic_circulant;
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

    // Wait a few cycles for registers to stabilize (solves problem with reading back 0s)
    tick();
    tick();
    
    // 1. Write the matrix (will be stored in a circulant pattern)
    cout << "\n1. Writing matrix:" << endl;
    dut->write_en = 1;
    for (int i = 0; i < 4; i++) {
        cout << "   Row " << i << ": [";
        for (int j = 0; j < 4; j++) {
            dut->write_row = i;
            dut->write_col = j;
            dut->data_in = 10*(i+1) + (j+1);  // Creates 11,12,13,14,21,22...
            cout << (int)dut->data_in;
            if (j < 3) cout << ", ";
            tick();
        }
        cout << "]" << endl;
    }
    dut->write_en = 0;
    
    tick();
    tick();

    // 2. Read back original matrix (should be transposed)
    cout << "\n2. Read in original direction:" << endl;
    dut->read_en = 1;
    
    for (int i = 0; i < 4; i++) {
        cout << "   Row " << i << ": [";
        for (int j = 0; j < 4; j++) {
            dut->read_row = i;
            dut->read_col = j;
            tick(); 
            tick();
            cout << (int)dut->data_out;
            if (j < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    
    dut->read_en = 0;

    tick();
    tick();

    // 3. Read back in transposed direction
    cout << "\n3. Read in transposed direction:" << endl;
    dut->read_en = 1;
    for (int j = 0; j < 4; j++) {
        cout << "   Row " << j << ": [";
        for (int i = 0; i < 4; i++) {
            dut->read_row = i;
            dut->read_col = j;
            tick(); 
            tick();
            cout << (int)dut->data_out;
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    dut->read_en = 0;
    tick();
    tick();
    
    cout << "\n=== TEST COMPLETE ===" << endl;

    delete dut;
    return 0;
}