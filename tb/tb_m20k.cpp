#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <random>
#include <map>
#include <bitset>
#include <verilated.h>
#include "Vm20k_bram_core.h"

class M20kTester {
private:
    Vm20k_bram_core* dut;
    vluint64_t sim_time;
    static const int PHYS_WIDTH = 160;
    static const int PHYS_DEPTH = 128;
    
    // Test configuration tracking
    int log_width;
    int log_depth;
    int test_count;
    int pass_count;
    int fail_count;
    
    // Reference memory for verification
    std::map<uint32_t, uint32_t> ref_memory;
    
public:
    M20kTester(int width = 8, int depth = 2048) : 
        sim_time(0), log_width(width), log_depth(depth), 
        test_count(0), pass_count(0), fail_count(0) {
            
        dut = new Vm20k_bram_core();
        reset_ports();
        std::cout << "=== M20K BRAM Tester Initialized ===" << std::endl;
        std::cout << "Configuration: " << log_width << "x" << log_depth << std::endl;
        std::cout << "Physical: " << PHYS_DEPTH << "x" << PHYS_WIDTH << std::endl;
    }

    ~M20kTester() {
        delete dut;
        print_summary();
    }

    void reset_ports() {
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

    void tick() {
        dut->clk = 0;
        dut->eval();
        dut->clk = 1;
        dut->eval();
        dut->clk = 0;
        dut->eval();
        sim_time++;
    }

    void tick(int n) {
        for (int i = 0; i < n; i++) tick();
    }

    void dut_reset() {
        dut->rst = 1;
        tick();
        dut->rst = 0;
        tick();
        // ref_memory.clear();  // Reference memeory is not cleared in HW
    }

    // Test result tracking
    void assert_test(bool condition, const std::string& test_name, const std::string& details = "") {
        test_count++;
        if (condition) {
            pass_count++;
            std::cout << "[PASS] " << test_name;
            if (!details.empty()) std::cout << " - " << details;
            std::cout << std::endl;
        } else {
            fail_count++;
            std::cout << "[FAIL] " << test_name;
            if (!details.empty()) std::cout << " - " << details;
            std::cout << std::endl;
        }
    }

    void print_summary() {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Total Tests: " << test_count << std::endl;
        std::cout << "Passed: " << pass_count << std::endl;
        std::cout << "Failed: " << fail_count << std::endl;
        std::cout << "Success Rate: " << (test_count > 0 ? (100.0 * pass_count / test_count) : 0) << "%" << std::endl;
    }

    void print_memory() {
        std::cout << "\n=== Reference Memory State ===" << std::endl;
        for (const auto& entry : ref_memory) {
            std::cout << "Addr: " << std::dec << entry.first 
                      << ", Data: 0x" << std::bitset<8>(entry.second) << std::dec << std::endl;
        }
    }

    // =============== CORE TEST FUNCTIONS ===============

    // Test 1: Basic Single Port Write/Read
    void test_basic_single_port_rw() {
        std::cout << "\n--- Test 1: Basic Single Port Read/Write ---" << std::endl;
        dut_reset();
        
        // Test data patterns
        std::vector<uint32_t> test_addrs = {0, 1, log_depth/2, log_depth-1};
        std::vector<uint32_t> test_data = {0x00, 0xFF, 0xAA, 0x55};
        
        for (size_t i = 0; i < test_addrs.size(); i++) {
            uint32_t addr = test_addrs[i];
            uint32_t data = test_data[i] & ((1 << log_width) - 1); // Mask to data width
            
            // Write operation
            dut->addr_a = addr;
            dut->data_in_a = data;
            dut->wen_a = 1;
            dut->ren_a = 0;
            tick(2); // Allow for pipeline delay
            dut->wen_a = 0;
            
            // Read operation
            dut->ren_a = 1;
            dut->addr_a = addr;
            tick(2); // Allow for read latency
            dut->ren_a = 0;
            
            uint32_t read_data = dut->data_out_a & ((1 << log_width) - 1);
            assert_test(read_data == data, 
                       "Single port write/read addr=" + std::to_string(addr),
                       "wrote=0x" + to_hex(data) + ", read=0x" + to_hex(read_data));
            
            ref_memory[addr] = data; // Track in reference
        }
    }

    // Test 2: Address Boundary Testing
    void test_address_boundaries() {
        std::cout << "\n--- Test 2: Address Boundary Testing ---" << std::endl;
        dut_reset();
        
        std::vector<uint32_t> boundary_addrs = {0, 1, log_depth/4, log_depth/2, 
                                               3*log_depth/4, log_depth-2, log_depth-1};
        
        for (uint32_t addr : boundary_addrs) {
            uint32_t test_data = (addr * 0x13) & ((1 << log_width) - 1); // Simple pattern
            
            // Write
            write_port_a(addr, test_data);
            tick(1);
            
            // Read back
            uint32_t read_data = read_port_a(addr);
            
            assert_test(read_data == test_data,
                       "Boundary address test addr=" + std::to_string(addr),
                       "wrote=0x" + to_hex(test_data) + ", read=0x" + to_hex(read_data));
        }
    }

    // Test 3: Data Pattern Testing
    void test_data_patterns() {
        std::cout << "\n--- Test 3: Data Pattern Testing ---" << std::endl;
        dut_reset();
        
        uint32_t max_data = (1 << log_width) - 1;
        
        // Walking 1s pattern
        for (int bit = 0; bit < log_width; bit++) {
            uint32_t addr = bit;
            uint32_t data = 1 << bit;
            
            write_port_a(addr, data);
            tick(1);
            uint32_t read_data = read_port_a(addr);
            
            assert_test(read_data == data,
                       "Walking 1s bit " + std::to_string(bit),
                       "wrote=0x" + to_hex(data) + ", read=0x" + to_hex(read_data));
        }
        
        // Walking 0s pattern
        for (int bit = 0; bit < log_width; bit++) {
            uint32_t addr = log_width + bit;
            uint32_t data = max_data & ~(1 << bit);
            
            write_port_a(addr, data);
            tick(1);
            uint32_t read_data = read_port_a(addr);
            
            assert_test(read_data == data,
                       "Walking 0s bit " + std::to_string(bit),
                       "wrote=0x" + to_hex(data) + ", read=0x" + to_hex(read_data));
        }

        // ref_memory.clear(); // Clear memory during debug
        // Checkerboard patterns (repeating 1's and 0's)
        std::vector<uint32_t> patterns = {0xAA & max_data, 0x55 & max_data, 
                                         0xCC & max_data, 0x33 & max_data};
        for (size_t i = 0; i < patterns.size(); i++) {
            uint32_t addr = 2 * log_width + i;
            uint32_t data = patterns[i];
            
            write_port_a(addr, data);
            tick(1);
            uint32_t read_data = read_port_a(addr);
            
            assert_test(read_data == data,
                       "Checkerboard pattern " + std::to_string(i),
                       "wrote=0x" + to_hex(data) + ", read=0x" + to_hex(read_data));
        }
        // print_memory(); // Print memory for debug
    }

    // Test 4: Basic Dual Port Independent Access
    void test_dual_port_independent() {
        std::cout << "\n--- Test 4: Dual Port Independent Access ---" << std::endl;
        dut_reset();
        
        // Test simultaneous writes to different addresses
        uint32_t addr_a = 10;
        uint32_t addr_b = 20;
        uint32_t data_a = 0x12 & ((1 << log_width) - 1);
        uint32_t data_b = 0x34 & ((1 << log_width) - 1);
        
        // Simultaneous write - don't use helper functions for tighter timing control
        dut->addr_a = addr_a;
        dut->data_in_a = data_a;
        dut->wen_a = 1;
        dut->ren_a = 0;
        
        dut->addr_b = addr_b;
        dut->data_in_b = data_b;
        dut->wen_b = 1;
        dut->ren_b = 0;
        
        tick(2);
        
        dut->wen_a = 0;
        dut->wen_b = 0;
        
        // Simultaneous read - don't use helper functions for tighter timing control
        dut->ren_a = 1;
        dut->ren_b = 1;
        tick(2);
        uint32_t read_a = (int)dut->data_out_a;
        uint32_t read_b = (int)dut->data_out_b;
        
        assert_test(read_a == data_a,
                   "Dual port write Port A",
                   "wrote=0x" + to_hex(data_a) + ", read=0x" + to_hex(read_a));
        
        assert_test(read_b == data_b,
                   "Dual port write Port B", 
                   "wrote=0x" + to_hex(data_b) + ", read=0x" + to_hex(read_b));
    }

    // Test 5: Data Width and Truncation Testing
    void test_data_width_handling() {
        ref_memory.clear();
        std::cout << "\n--- Test 5: Data Width and Truncation Testing ---" << std::endl;
        dut_reset();
        
        uint32_t test_addr = 50;
        uint32_t data_mask = (1 << log_width) - 1;
        
        std::cout << "Testing data width: " << log_width << " bits (mask: 0x" 
                  << std::hex << data_mask << std::dec << ")" << std::endl;
        
        // Test 1: Write data that fits exactly in the logical width
        uint32_t exact_data = data_mask; // All 1s for the width
        write_port_a_raw(test_addr, exact_data); // Use raw write to avoid masking
        uint32_t read_exact = read_port_a(test_addr);
        
        assert_test(read_exact == exact_data,
                   "Exact width data (all 1s)",
                   "wrote=0x" + to_hex(exact_data) + ", read=0x" + to_hex(read_exact));
        
        // Test 2: Write data larger than logical width - test truncation
        std::vector<uint32_t> oversized_data = {
            0x100 | 0xAB,  // 9+ bits: should truncate to 0xAB (for 8-bit) or lower bits
            0x1234,        // 16+ bits: should truncate to lower bits
            0xABCD5678,    // 32 bits: should truncate to lowest bits
            0xFFFFFFFF     // All 1s in 32 bits: should truncate to data_mask
        };
        
        for (size_t i = 0; i < oversized_data.size(); i++) {
            uint32_t big_data = oversized_data[i];
            uint32_t expected = big_data & data_mask; // What we expect after truncation
            
            test_addr++; // Use different address for each test
            write_port_a_raw(test_addr, big_data);
            uint32_t read_back = read_port_a(test_addr);
            
            assert_test(read_back == expected,
                       "Oversized data truncation test " + std::to_string(i),
                       "wrote=0x" + to_hex(big_data) + ", expected=0x" + to_hex(expected) + 
                       ", read=0x" + to_hex(read_back));
        }
        
        // Test 3: Test upper bits are properly ignored
        if (log_width < 32) {
            uint32_t base_pattern = 0x55 & data_mask;
            uint32_t with_upper_bits = base_pattern | (0xDEADBEEF << log_width);
            
            test_addr++;
            write_port_a_raw(test_addr, with_upper_bits);
            uint32_t read_upper = read_port_a(test_addr);
            
            assert_test(read_upper == base_pattern,
                       "Upper bits ignored test",
                       "wrote=0x" + to_hex(with_upper_bits) + ", expected=0x" + 
                       to_hex(base_pattern) + ", read=0x" + to_hex(read_upper));
        }
        
        // Test 4: Test both ports handle width consistently
        test_addr++;
        uint32_t dual_test_data = 0x12345678;
        uint32_t expected_dual = dual_test_data & data_mask;
        
        write_port_a_raw(test_addr, dual_test_data);
        write_port_b_raw(test_addr + 1, dual_test_data);
        
        uint32_t read_a = read_port_a(test_addr);
        uint32_t read_b = read_port_b(test_addr + 1);
        
        assert_test(read_a == expected_dual && read_b == expected_dual,
                   "Dual port width consistency",
                   "Port A: wrote=0x" + to_hex(dual_test_data) + ", read=0x" + to_hex(read_a) +
                   ", Port B: read=0x" + to_hex(read_b));
    }

    // Test 6: Same Address Access (Critical Edge Case)
    void test_same_address_access() {
        std::cout << "\n--- Test 6: Same Address Access ---" << std::endl;
        dut_reset();
        
        uint32_t addr = 100;
        uint32_t data_a = 0x77 & ((1 << log_width) - 1);
        uint32_t data_b = 0x88 & ((1 << log_width) - 1);
        
        // Write initial value with Port A
        write_port_a(addr, data_a);      
        uint32_t initial_read = read_port_a(addr);
        assert_test(initial_read == data_a,
                   "Initial write before collision test",
                   "wrote=0x" + to_hex(data_a) + ", read=0x" + to_hex(initial_read));
        
        // Test simultaneous read/write to same address
        std::cout << "Testing simultaneous read/write collision..." << std::endl;
        
        dut->addr_a = addr;
        dut->wen_a = 1;
        dut->ren_a = 0;
        dut->data_in_a = data_b;
        
        dut->addr_b = addr;
        dut->wen_b = 0;
        dut->ren_b = 1;
        
        tick(2); // Allow collision to occur
        
        dut->wen_a = 0;
        dut->ren_b = 0;
        
        // Check what Port B read during collision - we expect this to be the value before the write happens
        uint32_t collision_read = dut->data_out_b & ((1 << log_width) - 1);
        std::cout << "Collision read result: 0x" << std::hex << collision_read << std::dec << std::endl;
        
        // Verify the write took effect
        tick(1);
        uint32_t final_read = read_port_b(addr);
        assert_test(final_read == data_b,
                   "Write during collision",
                   "wrote=0x" + to_hex(data_b) + ", read=0x" + to_hex(final_read));
    }

    // =============== HELPER FUNCTIONS ===============
    
    void write_port_a(uint32_t addr, uint32_t data) {
        // Mask data to logical width for normal operations
        uint32_t masked_data = data & ((1 << log_width) - 1);
        dut->addr_a = addr;
        dut->data_in_a = masked_data;
        dut->wen_a = 1;
        dut->ren_a = 0;
        tick(1);
        dut->wen_a = 0;
        ref_memory[addr] = masked_data;
    }
    
    void write_port_b(uint32_t addr, uint32_t data) {
        // Mask data to logical width for normal operations
        uint32_t masked_data = data & ((1 << log_width) - 1);
        dut->addr_b = addr;
        dut->data_in_b = masked_data;
        dut->wen_b = 1;
        dut->ren_b = 0;
        tick(2);
        dut->wen_b = 0;
        ref_memory[addr] = masked_data;
    }
    
    // Raw write functions for testing data width handling (no masking)
    void write_port_a_raw(uint32_t addr, uint32_t data) {
        dut->addr_a = addr;
        dut->data_in_a = data; // No masking - let the hardware handle it
        dut->wen_a = 1;
        dut->ren_a = 0;
        tick(2);
        dut->wen_a = 0;
        // Store the expected masked value in reference
        ref_memory[addr] = data & ((1 << log_width) - 1);
    }
    
    void write_port_b_raw(uint32_t addr, uint32_t data) {
        dut->addr_b = addr;
        dut->data_in_b = data; // No masking - let the hardware handle it
        dut->wen_b = 1;
        dut->ren_b = 0;
        tick(2);
        dut->wen_b = 0;
        // Store the expected masked value in reference
        ref_memory[addr] = data & ((1 << log_width) - 1);
    }
    
    uint32_t read_port_a(uint32_t addr, bool raw = false) {
        dut->addr_a = addr;
        dut->ren_a = 1;
        dut->wen_a = 0;
        tick(2);
        dut->ren_a = 0;
        if (raw) {
            return dut->data_out_a; // If raw, don't mask
        }
        return dut->data_out_a & ((1 << log_width) - 1);
    }
    
    uint32_t read_port_b(uint32_t addr, bool raw = false) {
        dut->addr_b = addr;
        dut->ren_b = 1;
        dut->wen_b = 0;
        tick(2);
        dut->ren_b = 0;
        if (raw) {
            return dut->data_out_b; // If raw, don't mask
        }
        return dut->data_out_b & ((1 << log_width) - 1);
    }
    
    std::string to_hex(uint32_t value) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw((log_width + 3) / 4) << value;
        return ss.str();
    }

    // =============== TEST RUNNER ===============
    
    void run_core_tests() {
        std::cout << "Starting Core Functionality Tests..." << std::endl;
        
        test_basic_single_port_rw();
        test_address_boundaries();  
        test_data_patterns();
        test_dual_port_independent();
        test_data_width_handling();
        test_same_address_access();
        
        std::cout << "\nCore tests completed!" << std::endl;
    }
};

// Test different memory configurations
void test_memory_configurations() {
    std::cout << "\n\n=============== TESTING MEMORY CONFIGURATIONS ===============" << std::endl;
    
    // Configuration 1: 8x2048 (default)
    {
        std::cout << "\n### Testing 8x2048 Configuration ###" << std::endl;
        M20kTester tester_8x2048(8, 2048);
        tester_8x2048.run_core_tests();
    }
    
    // Note: Testing different logical configs means running verilator to recompile
    std::cout << "\nNote: To test 40x512 and 4x4096 configurations," << std::endl;
    std::cout << "recompile Verilog with LOGICAL_DATA_WIDTH and LOGICAL_DEPTH parameters." << std::endl;
}

int main() {
    std::cout << "M20K BRAM Comprehensive Test Suite" << std::endl;
    std::cout << "===================================" << std::endl;
    
    test_memory_configurations();
    
    std::cout << "\n=== All Tests Complete ===" << std::endl;
    return 0;
}