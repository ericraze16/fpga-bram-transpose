#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <verilated.h>
#include "Vm20k_bram_core.h"

class M20kTester {
public:
    Vm20k_bram_core* dut;
    vluint64_t sim_time;
    static const int PHYS_WIDTH = 160;
    static const int PHYS_DEPTH = 128;
    int log_width = 8;
    int log_depth = 2048;

// public:
    M20kTester() : sim_time(0) {
        dut = new Vm20k_bram_core();
        dut->clk = 0;
        dut->rst = 0;

        dut->addr_a = 0;
        dut->data_in_a = 0;
        dut->wen_a = 0;
        dut->ren_a = 0;

        dut->addr_b = 0;
        dut->data_in_b = 0;
        dut->wen_b = 0;
        dut->ren_b = 0;
    }

    ~M20kTester() {
        delete dut;
    }

    void tick() {
        dut->clk = 0;
        dut->eval();
        dut->clk = 1;
        dut->eval();
        dut->clk = 0;
        dut->eval();
        sim_time++;
    }

    void dut_reset() {
        dut->rst = 1;
        tick();
        dut->rst = 0;
        tick();
    }
};

int main() {
    M20kTester tester;

    // Ensure data out is 0
    tester.dut_reset();

    // Write a val
    tester.dut->addr_a = 0;
    tester.dut->data_in_a = 12; // random value 
    tester.dut->wen_a = 1;
    tester.dut->ren_a = 0; 
    tester.tick();
    tester.dut->wen_a = 0;

    // Read back the value
    tester.dut->ren_a = 1;    
    tester.dut->addr_a = 0; // Read from the same address
    tester.tick();
    tester.tick();
    std::cout << "Read data: 0x"<< std::hex << static_cast<unsigned int>(tester.dut->data_out_a) << std::dec << std::endl;
    tester.dut->ren_a = 0;

    return 0;
}