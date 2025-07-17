# fpga-bram-transpose
Research + developing in-memory (BRAM) transpose hardware

## Project Structure

```plaintext
fpga-transpose-research/
├── rtl/                    # RTL source files
│   ├── baseline/          # Conventional implementations
│   ├── optimized/         # Research optimizations
│   └── common/            # Shared modules
├── tb/                    # Testbenches
├── constraints/           # Timing and placement constraints
├── scripts/               # Build and analysis scripts
├── results/               # Synthesis and implementation results
├── docs/                  # Documentation and research notes
└── build/                 # Build artifacts (ignored by git)
```


## Quick Start (OUTDATED)
1. Install verilator
2. Compile RTL into object files: `make run_verilator`
  Use MATRIX_DIM=X to set the HW matrix size paramter. This must be set at compile time. The test bench must not try to write data larger than the HW size.
4. Build executable: `make build`
5. Run `make run`

## Dependencies
- Verilator
