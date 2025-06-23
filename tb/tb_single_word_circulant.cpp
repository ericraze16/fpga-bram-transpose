// Verilator C++ testbench for single_word_circulant
// Save as: tb_single_word_circulant.cpp

#include <iostream>
#include <iomanip>
#include <verilated.h>
#include "Vsingle_word_circulant.h" 

using namespace std;

// Test parameters - should match your module parameters
const int MATRIX_DIM = 4;
const int COL_WIDTH = 8;
const int WORD_LEN = 32;
const int COLS_PER_WORD = WORD_LEN / COL_WIDTH; // 4 columns per word

int main(int argc, char** argv) {
    // Initialize
    Verilated::commandArgs(argc, argv);
    Vsingle_word_circulant* dut = new Vsingle_word_circulant;
    dut->clk = 0;
    dut->write_en = 0;
    dut->read_en = 0;
    
    cout << "=== SINGLE WORD CIRCULANT MATRIX TEST ===" << endl;
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

    // Helper function to create test data word from 4 column values
    auto make_word = [](uint8_t col0, uint8_t col1, uint8_t col2, uint8_t col3) -> uint32_t {
        return (uint32_t(col3) << 24) | (uint32_t(col2) << 16) | (uint32_t(col1) << 8) | uint32_t(col0);
    };

    // Helper function to extract column from word
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
        // Create a word with distinguishable values for each column
        // Format: Row(row+1)Col(col+1) -> 11,12,13,14 for row 0
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
            cout << (int)col_vals[(row + i) % COLS_PER_WORD];
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
        
        tick();
    }

    // Print data pretty
    cout << endl << "Written words (non-circulant data layout):" << endl;
    for (int row = 0; row < MATRIX_DIM; row++) {
        uint8_t col_vals[COLS_PER_WORD];
        for (int col = 0; col < COLS_PER_WORD; col++) {
            col_vals[col] = 10 * (row + 1) + (col + 1);
        }
        cout << "   Row " << row;
        cout << " -> [" << dec;
        for (int i = 0; i < 4; i++) {
            cout << (int)col_vals[i];
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    dut->write_en = 0;
    
    // Wait for writes to complete
    tick();
    tick();

    // 2. Test regular reads (should get back original data, but transposed due to circulant storage)
    cout << "\n2. Transposed data (note: this will be circulant):" << endl;
    dut->read_en = 1;

    uint32_t read_words[MATRIX_DIM];
    
    for (int row = 0; row < MATRIX_DIM; row++) {
        dut->read_col = (row % COLS_PER_WORD); // The start chunk gets indented 1 chunk each row down
        dut->read_row = row; // Read full word starting at column 0
        
        tick(); // Clock the read
        tick(); // Extra cycle for read latency
        
        uint32_t read_word = dut->data_out;
        read_words[row] = read_word; // Store for later verification
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

    // Print the read words for verification
    cout << "\nRead words (non-circulant layout):" << endl;
    for (int row = 0; row < MATRIX_DIM; row++) {
        cout << "   Row " << row;
        cout << " -> [" << dec;
        for (int i = 0; i < 4; i++) {
            cout << (int)extract_col(read_words[row], (i + MATRIX_DIM-row) % COLS_PER_WORD);
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    /*

    // 3. Test reading with different starting columns to verify chunking
    cout << "\n3. Testing reads with different starting columns:" << endl;
    dut->read_en = 1;
    
    for (int start_col = 0; start_col < MATRIX_DIM; start_col++) {
        cout << "   Starting at column " << start_col << ":" << endl;
        
        for (int row = 0; row < MATRIX_DIM; row++) {
            dut->read_row = row;
            dut->read_col = start_col;
            
            tick();
            tick();
            
            uint32_t read_word = dut->data_out;
            cout << "     Row " << row << ": [";
            for (int i = 0; i < 4; i++) {
                cout << (int)extract_col(read_word, i);
                if (i < 3) cout << ", ";
            }
            cout << "]" << endl;
        }
        cout << endl;
    }
    
    dut->read_en = 0;
    tick();
    tick();

    // 4. Verify transpose functionality by writing a simple test pattern
    cout << "\n4. Transpose verification test:" << endl;
    cout << "   Writing identity-like pattern and checking transpose..." << endl;
    
    dut->write_en = 1;
    
    // Write a pattern where we can easily verify transposition
    // Row 0: [1, 0, 0, 0], Row 1: [0, 2, 0, 0], etc.
    for (int row = 0; row < MATRIX_DIM; row++) {
        uint8_t col_vals[4] = {0, 0, 0, 0};
        col_vals[row] = row + 1; // Put unique value in diagonal position
        
        uint32_t test_word = make_word(col_vals[0], col_vals[1], col_vals[2], col_vals[3]);
        
        dut->write_row = row;
        dut->write_col = 0;
        dut->data_in = test_word;
        
        cout << "   Writing row " << row << ": [";
        for (int i = 0; i < 4; i++) {
            cout << (int)col_vals[i];
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
        
        tick();
    }
    dut->write_en = 0;
    
    tick();
    tick();
    
    // Read back and show the transpose effect
    cout << "   Reading back (transposed):" << endl;
    dut->read_en = 1;
    
    for (int row = 0; row < MATRIX_DIM; row++) {
        dut->read_row = row;
        dut->read_col = 0;
        
        tick();
        tick();
        
        uint32_t read_word = dut->data_out;
        cout << "   Read row " << row << ": [";
        for (int i = 0; i < 4; i++) {
            cout << (int)extract_col(read_word, i);
            if (i < 3) cout << ", ";
        }
        cout << "]" << endl;
    }
    
    dut->read_en = 0;
    tick();
    tick();

    // 5. Test edge cases
    cout << "\n5. Edge case testing:" << endl;
    
    // Test writing and reading at maximum addresses
    cout << "   Testing maximum address values..." << endl;
    dut->write_en = 1;
    dut->write_row = MATRIX_DIM - 1;
    dut->write_col = MATRIX_DIM - 1;
    dut->data_in = 0xDEADBEEF;
    tick();
    dut->write_en = 0;
    
    tick();
    tick();
    
    dut->read_en = 1;
    dut->read_row = MATRIX_DIM - 1;
    dut->read_col = MATRIX_DIM - 1;
    tick();
    tick();
    
    cout << "   Wrote 0xDEADBEEF, read back 0x" << hex << setfill('0') << setw(8) << dut->data_out << dec << endl;
    */
    dut->read_en = 0;
    tick();
    tick();
    
    cout << "\n=== TEST COMPLETE ===" << endl;
    cout << "\nNotes:" << endl;
    cout << "- The circulant storage pattern causes data to appear transposed" << endl;
    cout << "- Wide words are automatically chunked across multiple columns" << endl;
    cout << "- Read operations increment row for each chunk (transpose behavior)" << endl;
    cout << "- Column addressing wraps around due to circulant pattern" << endl;

    delete dut;
    return 0;
}