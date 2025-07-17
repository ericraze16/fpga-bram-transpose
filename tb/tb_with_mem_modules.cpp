/*
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

        wait_cycles(5); 
        
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
} */


#include <iostream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <verilated.h>
#include "Vcirculant_barrel_shifter_v2.h"

// Template-based test class for different matrix dimensions
template<int MATRIX_DIM, int MEM_WIDTH = 8>
class CirculantShifterTester {
private:
    Vcirculant_barrel_shifter_v2* dut;
    vluint64_t sim_time;
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
    std::vector<uint8_t> row_to_elements(uint64_t row_data) {
        std::vector<uint8_t> elements;
        for (int i = 0; i < MATRIX_DIM; i++) {
            elements.push_back((row_data >> (i * MEM_WIDTH)) & ((1 << MEM_WIDTH) - 1));
        }
        return elements;
    }
    
    // Helper to convert individual elements to row data
    uint64_t elements_to_row(const std::vector<uint8_t>& elements) {
        uint64_t row_data = 0;
        for (int i = 0; i < MATRIX_DIM; i++) {
            row_data |= (uint64_t(elements[i]) << (i * MEM_WIDTH));
        }
        return row_data;
    }
    
    // Print row data in a readable format
    void print_row(uint64_t row_data, const std::string& label) {
        auto elements = row_to_elements(row_data);
        std::cout << label << ": [";
        for (int i = 0; i < MATRIX_DIM; i++) {
            std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                      << (int)elements[i];
            if (i < MATRIX_DIM - 1) std::cout << ", ";
        }
        std::cout << "]" << std::dec << std::endl;
    }
    
    // Print entire matrix for visualization
    void print_matrix(const std::vector<std::vector<uint8_t>>& matrix, const std::string& title) {
        std::cout << "\n" << title << " (" << MATRIX_DIM << "x" << MATRIX_DIM << "):" << std::endl;
        for (int row = 0; row < MATRIX_DIM; row++) {
            std::cout << "Row " << row << ": [";
            for (int col = 0; col < MATRIX_DIM; col++) {
                std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                          << (int)matrix[row][col];
                if (col < MATRIX_DIM - 1) std::cout << ", ";
            }
            std::cout << "]" << std::dec << std::endl;
        }
    }
    
    // Write a row to the matrix
    void write_row(uint8_t row_addr, const std::vector<uint8_t>& data) {
        assert(data.size() == MATRIX_DIM);
        
        uint64_t row_data = elements_to_row(data);
        
        posedge();
        print_time();
        std::cout << "Writing to row " << (int)row_addr << std::endl;
        print_row(row_data, " Data");
        std::cout << "  Address: " << (int)row_addr << std::endl;
        
        dut->waddr = row_addr;
        dut->wdata = row_data;
        dut->wen = 1;
        
        posedge();
        
        dut->wen = 0;
        posedge();
    }
    
    // Read a transformed row from the matrix
    uint64_t read_transformed_row(uint8_t transform_addr) {
        std::cout << "Reading transformed row with addr " << (int)transform_addr << " ";
        
        dut->rTransAddr = transform_addr;
        dut->ren = 1;

        wait_cycles(10); 
        
        uint64_t result = dut->rTransData;
        print_row(result, "  Result");
        
        return result;
    }
    
    // Generate test matrix with different patterns
    std::vector<std::vector<uint8_t>> generate_test_matrix(const std::string& pattern) {
        std::vector<std::vector<uint8_t>> matrix(MATRIX_DIM, std::vector<uint8_t>(MATRIX_DIM));
        
        if (pattern == "identity") {
            for (int i = 0; i < MATRIX_DIM; i++) {
                for (int j = 0; j < MATRIX_DIM; j++) {
                    matrix[i][j] = (i == j) ? 1 : 0;
                }
            }
        } else if (pattern == "sequential") {
            uint8_t val = 1;
            for (int i = 0; i < MATRIX_DIM; i++) {
                for (int j = 0; j < MATRIX_DIM; j++) {
                    matrix[i][j] = val++;
                }
            }
        } else if (pattern == "row_distinct") {
            for (int i = 0; i < MATRIX_DIM; i++) {
                for (int j = 0; j < MATRIX_DIM; j++) {
                    matrix[i][j] = (i + 1) * 0x10 + j;
                }
            }
        } else if (pattern == "alternating") {
            for (int i = 0; i < MATRIX_DIM; i++) {
                for (int j = 0; j < MATRIX_DIM; j++) {
                    matrix[i][j] = ((i + j) % 2) ? 0xAA : 0x55;
                }
            }
        } else if (pattern == "diagonal") {
            for (int i = 0; i < MATRIX_DIM; i++) {
                for (int j = 0; j < MATRIX_DIM; j++) {
                    matrix[i][j] = (i + j) % MATRIX_DIM + 1;
                }
            }
        } else if (pattern == "random_like") {
            // Pseudo-random pattern for testing
            for (int i = 0; i < MATRIX_DIM; i++) {
                for (int j = 0; j < MATRIX_DIM; j++) {
                    matrix[i][j] = (i * 7 + j * 13) % 256;
                }
            }
        }
        
        return matrix;
    }
    
    // Expected transpose for verification
    std::vector<std::vector<uint8_t>> transpose_matrix(const std::vector<std::vector<uint8_t>>& matrix) {
        std::vector<std::vector<uint8_t>> transposed(MATRIX_DIM, std::vector<uint8_t>(MATRIX_DIM));
        for (int i = 0; i < MATRIX_DIM; i++) {
            for (int j = 0; j < MATRIX_DIM; j++) {
                transposed[j][i] = matrix[i][j];
            }
        }
        return transposed;
    }
    
    // Test with different matrix patterns
    void test_matrix_pattern(const std::string& pattern) {
        std::cout << "\n=== Testing " << pattern << " Pattern (" << MATRIX_DIM << "x" << MATRIX_DIM << ") ===" << std::endl;
        
        auto test_matrix = generate_test_matrix(pattern);
        auto expected_transpose = transpose_matrix(test_matrix);
        
        print_matrix(test_matrix, "Original Matrix");
        print_matrix(expected_transpose, "Expected Transpose");
        
        // Write the test matrix
        for (int row = 0; row < MATRIX_DIM; row++) {
            write_row(row, test_matrix[row]);
        }
        
        // Wait for writes to settle
        wait_cycles(10);
        
        // Read back and verify transpose
        std::cout << "\nReading transposed data:" << std::endl;
        std::vector<std::vector<uint8_t>> actual_transpose(MATRIX_DIM, std::vector<uint8_t>(MATRIX_DIM));
        
        for (int transform = 0; transform < MATRIX_DIM; transform++) {
            uint64_t result = read_transformed_row(transform);
            auto result_elements = row_to_elements(result);
            actual_transpose[transform] = result_elements;
        }
        
        print_matrix(actual_transpose, "Actual Transpose");
        
        // Verify correctness
        bool correct = true;
        for (int i = 0; i < MATRIX_DIM; i++) {
            for (int j = 0; j < MATRIX_DIM; j++) {
                if (actual_transpose[i][j] != expected_transpose[i][j]) {
                    correct = false;
                    std::cout << "MISMATCH at [" << i << "][" << j << "]: expected 0x" 
                              << std::hex << (int)expected_transpose[i][j] 
                              << ", got 0x" << (int)actual_transpose[i][j] << std::dec << std::endl;
                }
            }
        }
        
        if (correct) {
            std::cout << "✓ " << pattern << " pattern test PASSED" << std::endl;
        } else {
            std::cout << "✗ " << pattern << " pattern test FAILED" << std::endl;
        }
    }
    
    // Test sparse write/read patterns
    void test_sparse_operations() {
        std::cout << "\n=== Testing Sparse Operations (" << MATRIX_DIM << "x" << MATRIX_DIM << ") ===" << std::endl;
        
        // Write only odd rows
        std::vector<uint8_t> odd_row_data(MATRIX_DIM);
        for (int row = 1; row < MATRIX_DIM; row += 2) {
            for (int col = 0; col < MATRIX_DIM; col++) {
                odd_row_data[col] = row * 0x10 + col;
            }
            write_row(row, odd_row_data);
        }
        
        wait_cycles(10);
        
        // Read all transforms
        std::cout << "Reading after sparse writes:" << std::endl;
        for (int transform = 0; transform < MATRIX_DIM; transform++) {
            read_transformed_row(transform);
        }
    }
    
    // Test write-read interleaving
    void test_interleaved_operations() {
        std::cout << "\n=== Testing Interleaved Write/Read (" << MATRIX_DIM << "x" << MATRIX_DIM << ") ===" << std::endl;
        
        auto test_matrix = generate_test_matrix("sequential");
        
        // Interleave writes and reads
        for (int i = 0; i < MATRIX_DIM; i++) {
            write_row(i, test_matrix[i]);
            wait_cycles(2);
            
            // Read what we can so far
            if (i > 0) {
                read_transformed_row(i - 1);
            }
        }
        
        // Final read pass
        wait_cycles(10);
        std::cout << "Final read pass:" << std::endl;
        for (int transform = 0; transform < MATRIX_DIM; transform++) {
            read_transformed_row(transform);
        }
    }
    
    // Test boundary conditions
    void test_boundary_conditions() {
        std::cout << "\n=== Testing Boundary Conditions (" << MATRIX_DIM << "x" << MATRIX_DIM << ") ===" << std::endl;
        
        // Test with zero data
        std::vector<uint8_t> zero_row(MATRIX_DIM, 0x00);
        write_row(0, zero_row);
        wait_cycles(2);
        read_transformed_row(0);
        
        // Test with maximum values
        std::vector<uint8_t> max_row(MATRIX_DIM, 0xFF);
        write_row(MATRIX_DIM - 1, max_row);
        wait_cycles(2);
        read_transformed_row(MATRIX_DIM - 1);
        
        // Test corner addressing
        std::vector<uint8_t> corner_row(MATRIX_DIM);
        for (int i = 0; i < MATRIX_DIM; i++) {
            corner_row[i] = (i == 0 || i == MATRIX_DIM - 1) ? 0xFF : 0x00;
        }
        write_row(MATRIX_DIM / 2, corner_row);
        wait_cycles(2);
        read_transformed_row(MATRIX_DIM / 2);
    }
    
    // Run comprehensive tests
    void run_all_tests() {
        std::cout << "Starting Comprehensive Circulant Barrel Shifter Tests" << std::endl;
        std::cout << "Matrix Dimension: " << MATRIX_DIM << std::endl;
        std::cout << "Memory Width: " << MEM_WIDTH << " bits" << std::endl;
        std::cout << "Row Width: " << ROW_WIDTH << " bits" << std::endl;
        
        // Reset and initialize
        wait_cycles(10);
        
        // Test different patterns
        test_matrix_pattern("identity");
        // test_matrix_pattern("sequential");
        test_matrix_pattern("row_distinct");
        // test_matrix_pattern("alternating");
        // test_matrix_pattern("diagonal");
        // test_matrix_pattern("random_like");
        
        // Test operational patterns
        // test_sparse_operations();
        // test_interleaved_operations();
        // test_boundary_conditions();
        
        std::cout << "\n=== All Tests Completed for " << MATRIX_DIM << "x" << MATRIX_DIM << " Matrix ===" << std::endl;
    }
};

// Test runner for multiple matrix sizes
int main(int argc, char** argv) {
    // Initialize Verilator
    Verilated::commandArgs(argc, argv);
    
    std::cout << "Running Circulant Barrel Shifter Tests for Multiple Matrix Sizes\n" << std::endl;
    
    // // Test 2x2 matrix
    // {
    //     std::cout << "\n" << std::string(60, '=') << std::endl;
    //     std::cout << "TESTING 2x2 MATRIX" << std::endl;
    //     std::cout << std::string(60, '=') << std::endl;
    //     CirculantShifterTester<2> tester_2x2;
    //     tester_2x2.run_all_tests();
    // }
    
    // // Test 4x4 matrix
    // {
    //     std::cout << "\n" << std::string(60, '=') << std::endl;
    //     std::cout << "TESTING 4x4 MATRIX" << std::endl;
    //     std::cout << std::string(60, '=') << std::endl;
    //     CirculantShifterTester<4> tester_4x4;
    //     tester_4x4.run_all_tests();
    // }

    // Test 3x3 matrix
    // {
    //     std::cout << "\n" << std::string(60, '=') << std::endl;
    //     std::cout << "TESTING 3x3 MATRIX" << std::endl;
    //     std::cout << std::string(60, '=') << std::endl;
    //     CirculantShifterTester<3> tester_3x3;
    //     tester_3x3.run_all_tests();
    // }
    
    // Test 5x5 matrix
    {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "TESTING 5x5 MATRIX" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        CirculantShifterTester<6, 8> tester_5x5;
        tester_5x5.run_all_tests();
    }
    
    // // Test 16x16 matrix (if your design supports it)
    // {
    //     std::cout << "\n" << std::string(60, '=') << std::endl;
    //     std::cout << "TESTING 16x16 MATRIX" << std::endl;
    //     std::cout << std::string(60, '=') << std::endl;
    //     CirculantShifterTester<16> tester_16x16;
    //     tester_16x16.run_all_tests();
    // }
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "ALL MATRIX SIZE TESTS COMPLETED" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    return 0;
}
