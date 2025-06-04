# FPGA Transpose Research Makefile

# Directories
RTL_DIR = rtl
TB_DIR = tb
BUILD_DIR = build
RESULTS_DIR = results

# Tools
VERILATOR = verilator
IVERILOG = iverilog
VVP = vvp
GTKWAVE = gtkwave

# Default target
.PHONY: all
all: sim

# Install dependencies (Ubuntu/Debian)
.PHONY: install-deps
install-deps:
	sudo apt update
	sudo apt install -y verilator iverilog gtkwave python3 python3-pip
	pip3 install matplotlib pandas numpy

# Clean build directory
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/*
	rm -f *.vcd

# Simulation targets
.PHONY: sim
sim: $(BUILD_DIR)/transpose_sim
	cd $(BUILD_DIR) && ./transpose_sim

# Compile with Verilator
$(BUILD_DIR)/transpose_sim: $(RTL_DIR)/baseline/matrix_transpose.v $(TB_DIR)/tb_matrix_transpose.v
	mkdir -p $(BUILD_DIR)
	$(VERILATOR) --cc --exe --build -o $@ \
		--trace \
		-I$(RTL_DIR)/baseline \
		-I$(RTL_DIR)/common \
		$(TB_DIR)/tb_matrix_transpose.v \
		$(RTL_DIR)/baseline/matrix_transpose.v

# Alternative: Icarus Verilog simulation
.PHONY: sim-iverilog
sim-iverilog: $(BUILD_DIR)/sim_iv
	cd $(BUILD_DIR) && $(VVP) sim_iv

$(BUILD_DIR)/sim_iv: $(RTL_DIR)/baseline/matrix_transpose.v $(TB_DIR)/tb_matrix_transpose.v
	mkdir -p $(BUILD_DIR)
	$(IVERILOG) -o $@ \
		-I$(RTL_DIR)/baseline \
		-I$(RTL_DIR)/common \
		$(TB_DIR)/tb_matrix_transpose.v \
		$(RTL_DIR)/baseline/matrix_transpose.v

# View waveforms
.PHONY: waves
waves: sim
	$(GTKWAVE) $(BUILD_DIR)/trace.vcd &

# Help
.PHONY: help
help:
	@echo "Available targets:"
	@echo "  install-deps  - Install required tools"
	@echo "  sim          - Run simulation with Verilator"
	@echo "  sim-iverilog - Run simulation with Icarus Verilog"
	@echo "  waves        - View waveforms with GTKWave"
	@echo "  clean        - Clean build directory"
	@echo "  help         - Show this help"
