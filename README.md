# fpga-bram-transpose
Research + developing in-memory (BRAM) transpose hardware

## Project Structure

```plaintext
fpga-transpose-research/
├── rtl/                   # RTL source files
│   ├── baseline/          # Conventional implementations (M20k model)
│   └── common/            # Shared modules (standard sram model)
├── tb/                    # Testbenches - contain automated tests for each model
```

## This repo contains 3 models:
1. Baseline implementation of a current transpose engine (`rtl/baseline/circulant_barrel_shifter_v2.v`). This is a module that instantiates many BRAM submodules, and orchestrates circular reading/writing of data across each BRAM submodule. The key element here is that this transpose engine requires MANY BRAM submodules because it assumes they are standard BRAMs WITHOUT partial wordline capabilities and enhanced crossbars.
2. Comprehensive functional model of a M20k BRAM (`rtl/baseline/m20k_bram_core.v`). This module contains the robust functionality of a M20k BRAM: configurable width/depth, true dual port reading/writing, collision detection.
3. M20k BRAM model enhanced with internal transpose abilities - internally capable of storing data with a circulant pattern using partial wordlines and modified crossbars (`rtl/m20k_bram_partial_wordlines.v`). The enhanced logic has not been built yet - so far it is a duplicate of the M20k BRAM model.

## Quick Start
1. Install verilator

To run the transpose engine:
1. `make ver_transpose` Use `MATRIX_DIM=x` to change transpose engine size. A square matrix is always built. Default size is 4.
2. `make build_transpose` In the tb, select the appropriate tests for the chosen matrix size.
3. `make run_transpose`

To run the M20k BRAM model:
1. `make ver_ram` Set `LOG_WIDTH=x LOG_DEPTH=y` to change the logical configuration of the BRAM. See the module for supported options.
2. `make build_ram` In the tb, change the `LOG_WIDTH` and `LOG_DEPTH` consts to match the compiled configuration
3. `make run_ram`

## Dependencies
- Verilator
