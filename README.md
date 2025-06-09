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
1. Install dependencies: `make install-deps`
2. Run simulation: `make sim`
3. View waveforms: `make waves`

## Dependencies
- Verilator
- GTKWave
- Python 3.8+

## Usage
See [docs/getting-started.md](docs/getting-started.md) for detailed setup instructions. (OUTDATED)
