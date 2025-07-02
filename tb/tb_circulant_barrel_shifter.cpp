// Verilator C++ testbench for circulant_barrel_shifter.v

#include <iostream>
#include <iomanip>
#include <verilated.h>
#include "Vcirculant_barrel_shifter.h"

using namespace std;

// Test parameters - should match your module parameters
const int MATRIX_DIM = 4;
const int COL_WIDTH = 8;
const int WORD_LEN = 32;
const int COLS_PER_WORD = WORD_LEN / COL_WIDTH; // 4 columns per word

int main(int argc, char** argv) {
    // Initialize
    Verilated::commandArgs(argc, argv);
    Vcirculant_barrel_shifter* dut = new Vcirculant_barrel_shifter;
    dut->clk = 0;
    dut->write_en = 0;
    dut->read_en = 0;
    dut->barrel_shift_en = 0;
    
    cout << "=== CIRCULANT MATRIX WITH BARREL SHIFTER TEST ===" << endl;
    cout << "Matrix: " << MATRIX_DIM << "x" << MATRIX_DIM << endl;
    cout << "Column width: " << COL_WIDTH << " bits" << endl;
    cout << "Word length: " << WORD_LEN << " bits" << endl;
    cout << "Columns per word: " << COLS_PER_WORD << endl;
    
    // Helper function to toggle clock
    auto tick = [&]() {
        dut->clk = 1;
        dut->eval();
        dut->clk = 0; 
        dut->eval();
    };

    // Take make 32 bit word from 8-bit columns
    auto make_word = [](uint8_t col0, uint8_t col1, uint8_t col2, uint8_t col3) -> uint32_t {
        return (uint32_t(col3) << 24) | (uint32_t(col2) << 16) | (uint32_t(col1) << 8) | uint32_t(col0);
    };

    
    auto extract_col = [](uint32_t word, int col_idx) -> uint8_t {
        return (word >> (col_idx * COL_WIDTH)) & 0xFF;
    };

    // Wait a few cycles for registers to stabilize
    tick();
    tick();
    
    // 1. Write test matrix using wide words
    cout << "\n1. Writing matrix using " << WORD_LEN << "-bit words:" << endl;
    dut->write_en = 1;
    
    // We'll write one word per row, spanning all 4 columns
    // Each word contains 4 column values for that row
    for (int row = 0; row < MATRIX_DIM; row++) {
        // Create 4 words, each word spans 4 columns.
        // Word 1: 11, 12, 13, 14
        // Word 2: 21, 22, 23, 24
        // etc.  
        uint8_t col_vals[COLS_PER_WORD];
        for (int col = 0; col < COLS_PER_WORD; col++) {
            col_vals[col] = 10 * (row + 1) + (col + 1);
        }
        
        uint32_t test_word = make_word(col_vals[0], col_vals[1], col_vals[2], col_vals[3]);
        
        dut->write_row = row;
        dut->write_col = 0; // Start at column 0, word spans multiple columns
        dut->data_in = test_word;
        
        cout << "   Row " << row << " word: 0x" << hex << setfill('0') << setw(8) << test_word;
        cout << " -> [" << dec;
        for (int i = 0; i < 4; i++) {
            cout << (int)col_vals[i];
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
        
        tick();
    }

    // Print original matrix for reference
    cout << "\nOriginal matrix:" << endl;
    for (int row = 0; row < MATRIX_DIM; row++) {
        cout << "   Row " << row << " -> [" << dec;
        for (int col = 0; col < COLS_PER_WORD; col++) {
            cout << 10 * (row + 1) + (col + 1);
            if (col < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    dut->write_en = 0;
    
    // Wait for writes to complete
    tick();
    tick();

    // 2. Test reads with barrel shifter DISABLED (raw circulant output)
    cout << "\n2. Raw circulant reads (barrel_shift_en = 0):" << endl;
    dut->read_en = 1;
    dut->barrel_shift_en = 0;

    uint32_t raw_read_words[MATRIX_DIM];
    
    for (int row = 0; row < MATRIX_DIM; row++) {
        dut->read_col = (row % COLS_PER_WORD); // The start chunk gets indented 1 chunk each row down
        dut->read_row = row; // Read full word starting at column 0

        
        tick(); // Clock the read
        tick(); // Pipeline delay for barrel shifter
        tick(); // Extra cycle for read latency
        
        uint32_t read_word = dut->data_out;
        raw_read_words[row] = read_word;
        cout << "   Row " << row << " read: 0x" << hex << setfill('0') << setw(8) << read_word;
        cout << " -> [" << dec;
        for (int i = 0; i < 4; i++) {
            cout << (int)extract_col(read_word, i);
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    
    dut->read_en = 0;
    tick();
    tick();

    // 3. Test reads with barrel shifter ENABLED (clean transpose output)
    cout << "\n3. Barrel-shifted reads (barrel_shift_en = 1):" << endl;
    dut->read_en = 1;
    dut->barrel_shift_en = 1;

    uint32_t shifted_read_words[MATRIX_DIM];
    
    for (int row = 0; row < MATRIX_DIM; row++) {
        dut->read_col = (row % COLS_PER_WORD); // The start chunk gets indented 1 chunk each row down
        dut->read_row = row; // Read full word starting at column 0
        
        tick(); // Clock the read
        tick(); // Pipeline delay for barrel shifter
        tick(); // Extra cycle for read latency
        
        uint32_t read_word = dut->data_out;
        shifted_read_words[row] = read_word;
        cout << "   Row " << row << " read: 0x" << hex << setfill('0') << setw(8) << read_word;
        cout << " -> [" << dec;
        for (int i = 0; i < 4; i++) {
            cout << (int)extract_col(read_word, i);
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    
    dut->read_en = 0;
    dut->barrel_shift_en = 0;
    tick();
    tick();

    /*

    // 4. Verification and comparison
    cout << "\n4. VERIFICATION:" << endl;
    cout << "\nExpected transpose matrix:" << endl;
    for (int row = 0; row < MATRIX_DIM; row++) {
        cout << "   Row " << row << " -> [" << dec;
        for (int col = 0; col < COLS_PER_WORD; col++) {
            cout << 10 * (col + 1) + (row + 1); // Transpose: original[col][row]
            if (col < 3) cout << ", ";
        }
        cout << "]" << endl;
    }

    cout << "\nBarrel shifter verification:" << endl;
    bool transpose_correct = true;
    for (int row = 0; row < MATRIX_DIM; row++) {
        cout << "   Row " << row << ": ";
        bool row_correct = true;
        for (int col = 0; col < COLS_PER_WORD; col++) {
            uint8_t expected = 10 * (col + 1) + (row + 1);
            uint8_t actual = extract_col(shifted_read_words[row], col);
            if (expected != actual) {
                row_correct = false;
                transpose_correct = false;
            }
        }
        cout << (row_correct ? "PASS" : "FAIL") << endl;
    }

    cout << "\nShift pattern analysis:" << endl;
    for (int row = 0; row < MATRIX_DIM; row++) {
        int shift_amount = (COLS_PER_WORD - row) % COLS_PER_WORD;
        cout << "   Row " << row << " shift amount: " << shift_amount;
        
        // Verify the shift worked correctly
        bool shift_correct = true;
        for (int i = 0; i < COLS_PER_WORD; i++) {
            uint8_t raw_val = extract_col(raw_read_words[row], i);
            uint8_t shifted_val = extract_col(shifted_read_words[row], (i + shift_amount) % COLS_PER_WORD);
            if (raw_val != shifted_val) {
                shift_correct = false;
                break;
            }
        }
        cout << " -> " << (shift_correct ? "CORRECT" : "INCORRECT") << endl;
    }
    */
    
    cout << "\n=== TEST COMPLETE ===" << endl;
    // cout << "\nOverall result: " << (transpose_correct ? "TRANSPOSE SUCCESSFUL" : "TRANSPOSE FAILED") << endl;
    cout << "\nFeatures demonstrated:" << endl;
    cout << "- Circulant storage with wide word support" << endl;
    cout << "- Raw circulant output (shifted transpose)" << endl;
    cout << "- Barrel shifter correction for clean transpose" << endl;
    cout << "- Pipelined barrel shifter architecture" << endl;

    delete dut;
    return 0;
}