#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <verilated.h>
#include "Vcirculant_barrel_shifter_v2.h"

// Test class for the circulant barrel shifter
class CirculantShifterTester {
private:
    Vcirculant_barrel_shifter_v2* dut;
    vluint64_t sim_time;
    static const int MATRIX_DIM = 4;
    static const int MEM_WIDTH = 8;
    static const int ROW_WIDTH = MATRIX_DIM * MEM_WIDTH;
    
public:
    CirculantShifterTester() : sim_time(0) {
        dut = new Vcirculant_barrel_shifter_v2();
        dut->clk = 0;
        dut->wen = 0;
        dut->ren = 0;
        dut->wdata = 0;
        dut->waddr = 0;
        dut->rTransAddr = 0;
    }
    
    ~CirculantShifterTester() {
        delete dut;
    }
    
    // Clock edge helper
    void posedge() {
        dut->clk = 1;
        dut->eval();
        sim_time++;
        dut->clk = 0;
        dut->eval();
        sim_time++;
    }
    
    // Wait for specified number of clock cycles
    void wait_cycles(int cycles) {
        for (int i = 0; i < cycles; i++) {
            posedge();
        }
    }
    
    // Print current time
    void print_time() {
        std::cout << "Time: " << sim_time << " ";
    }
    
    // Helper to convert row data to individual elements
    std::vector<uint8_t> row_to_elements(uint32_t row_data) {
        std::vector<uint8_t> elements;
        for (int i = 0; i < MATRIX_DIM; i++) {
            elements.push_back((row_data >> (i * MEM_WIDTH)) & 0xFF);
        }
        return elements;
    }
    
    // Helper to convert individual elements to row data
    uint32_t elements_to_row(const std::vector<uint8_t>& elements) {
        uint32_t row_data = 0;
        for (int i = 0; i < MATRIX_DIM; i++) {
            row_data |= (uint32_t(elements[i]) << (i * MEM_WIDTH));
        }
        return row_data;
    }
    
    // Print row data in a readable format
    void print_row(uint32_t row_data, const std::string& label) {
        auto elements = row_to_elements(row_data);
        std::cout << label << ": [";
        for (int i = 0; i < MATRIX_DIM; i++) {
            std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                      << (int)elements[i];
            if (i < MATRIX_DIM - 1) std::cout << ", ";
        }
        std::cout << "]" << std::dec << std::endl;
    }
    
    // Write a row to the matrix
    void write_row(uint8_t row_addr, const std::vector<uint8_t>& data) {
        assert(data.size() == MATRIX_DIM);
        
        uint32_t row_data = elements_to_row(data);
        
        posedge();
        print_time();
        std::cout << "Writing to row " << (int)row_addr << std::endl;
        print_row(row_data, " Data");
        
        dut->waddr = row_addr;
        dut->wdata = row_data;
        dut->wen = 1;
        
        posedge();
        
        dut->wen = 0;
        posedge();
    }
    
    // Read a transformed row from the matrix
    uint32_t read_transformed_row(uint8_t transform_addr) {
        // print_time();
        std::cout << "Reading transformed row with addr " << (int)transform_addr << " ";
        
        dut->rTransAddr = transform_addr;
        dut->ren = 1;

        wait_cycles(6); 
        
        uint32_t result = dut->rTransData;
        print_row(result, "  Result");
        
        return result;
    }
    
    // Test basic write/read functionality
    void test_basic_operations() {
        std::cout << "\n=== Testing Basic Operations ===" << std::endl;
        
        // Test data: each row has distinct pattern
        std::vector<std::vector<uint8_t>> test_matrix = {
            {0x10, 0x11, 0x12, 0x13},  // Row 0
            {0x20, 0x21, 0x22, 0x23},  // Row 1
            {0x30, 0x31, 0x32, 0x33},  // Row 2
            {0x40, 0x41, 0x42, 0x43}   // Row 3
        };
        
        // Write the test matrix
        for (int row = 0; row < MATRIX_DIM; row++) {
            write_row(row, test_matrix[row]);
        }
        
        
        // Wait for writes to settle
        wait_cycles(10);
        

        // Read back with different transform addresses
        std::cout << "\nReading back with different transform addresses:" << std::endl;
        for (int transform = 0; transform < MATRIX_DIM; transform++) {
            uint32_t result = read_transformed_row(transform);
            
            // For a circulant matrix, transform_addr should give us a shifted version
            // std::cout << "Transform " << transform << " completed" << std::endl;
        }
    }
    
    // Test circulant property
    void test_circulant_property() {
        std::cout << "\n=== Testing Circulant Property ===" << std::endl;
        
        // Create a simple circulant matrix manually for verification
        // First row: [1, 2, 3, 4]
        // Should produce:
        // Row 0: [1, 2, 3, 4]
        // Row 1: [4, 1, 2, 3]  (right shift by 1)
        // Row 2: [3, 4, 1, 2]  (right shift by 2)
        // Row 3: [2, 3, 4, 1]  (right shift by 3)
        
        std::vector<uint8_t> base_row = {0x01, 0x02, 0x03, 0x04};
        
        // Write the base row
        write_row(0, base_row);
        
        // Wait for write to settle
        wait_cycles(2);
        
        // Read different transforms and verify circulant property
        for (int transform = 0; transform < MATRIX_DIM; transform++) {
            uint32_t result = read_transformed_row(transform);
            auto result_elements = row_to_elements(result);
            
            std::cout << "Transform " << transform << " verification:" << std::endl;
            std::cout << "  Expected pattern based on circulant property" << std::endl;
            
            // You can add specific verification logic here based on your
            // understanding of what the transform should produce
        }
    }
    
    // Test edge cases
    void test_edge_cases() {
        std::cout << "\n=== Testing Edge Cases ===" << std::endl;
        
        // Test with zero data
        std::vector<uint8_t> zero_row = {0x00, 0x00, 0x00, 0x00};
        write_row(0, zero_row);
        wait_cycles(2);
        read_transformed_row(0);
        
        // Test with maximum values
        std::vector<uint8_t> max_row = {0xFF, 0xFF, 0xFF, 0xFF};
        write_row(1, max_row);
        wait_cycles(2);
        read_transformed_row(1);
        
        // Test with alternating pattern
        std::vector<uint8_t> alt_row = {0xAA, 0x55, 0xAA, 0x55};
        write_row(2, alt_row);
        wait_cycles(2);
        read_transformed_row(2);
    }
    
    // Run all tests
    void run_tests() {
        std::cout << "Starting Circulant Barrel Shifter Tests" << std::endl;
        std::cout << "Matrix Dimension: " << MATRIX_DIM << std::endl;
        std::cout << "Memory Width: " << MEM_WIDTH << " bits" << std::endl;
        std::cout << "Row Width: " << ROW_WIDTH << " bits" << std::endl;
        
        // Reset and initialize
        wait_cycles(10);
        
        test_basic_operations();
        // test_circulant_property();
        // test_edge_cases();
        
        std::cout << "\n=== All Tests Completed ===" << std::endl;
    }
};

int main(int argc, char** argv) {
    // Initialize Verilator
    Verilated::commandArgs(argc, argv);
    
    // Create and run tests
    CirculantShifterTester tester;
    tester.run_tests();
    
    // Clean up
    return 0;
}
