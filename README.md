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
3. Build executable: `make build`
4. Run `make run`

## Dependencies
- Verilator
