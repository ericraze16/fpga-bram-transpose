# Let user optionally pass matrix dimension for the transpose engine
# If not set, uses the default defined in the rtl (4x4)
MATRIX_PARAM = $(if $(MATRIX_DIM),--GMATRIX_DIM=$(MATRIX_DIM),)

# Let user optionally pass logical data width and depth for the m20k bram model
# if not set, uses the default value from the rtl (8 x 2056)
# TODO: Enable error checking for valid configs
LOG_WIDTH_PARAM = $(if $(LOG_WIDTH),--GLOGICAL_DATA_WIDTH=$(LOG_WIDTH),)
LOG_DEPTH_PARAM = $(if $(LOG_DEPTH),--GLOGICAL_DEPTH=$(LOG_DEPTH),)

# rtl and tb for transpose engine model
VERILOG_SOURCES = ./rtl/baseline/circulant_barrel_shifter_v2.v $(MATRIX_PARAM) ./rtl/common/bram_mem.v
CPP_TESTBENCH = ./tb/tb_with_mem_modules.cpp

# rtl and tb for m20k model
RAM_MODEL_SOURCES = ./rtl/baseline/m20k_bram_core.v $(LOG_WIDTH_PARAM) $(LOG_DEPTH_PARAM)
RAM_MODEL_TESTBENCH = ./tb/tb_m20k.cpp

# Uses verilator to compile HDL design and c++ testbench into object files
ver_transpose: 
	@echo "Compiling with$(if $(MATRIX_DIM), MATRIX_DIM=$(MATRIX_DIM), default MATRIX_DIM)"
	verilator -cc $(VERILOG_SOURCES) --exe $(CPP_TESTBENCH)

ver_ram:
	@echo "Compiling RAM model with$(if $(LOG_WIDTH/DEPTH), LOG_WIDTH/DEPTH=$(LOG_WIDTH/DEPTH), default LOG_WIDTH/DEPTH)"
	verilator -cc $(RAM_MODEL_SOURCES) --exe $(RAM_MODEL_TESTBENCH)

# Use make to build an executable from the generated object files
build_transpose:
	make -C ./obj_dir/ -f Vcirculant_barrel_shifter_v2.mk Vcirculant_barrel_shifter_v2

build_ram:
	make -C ./obj_dir/ -f Vm20k_bram_core.mk Vm20k_bram_core

# Run the executables
run_transpose:
	./obj_dir/Vcirculant_barrel_shifter_v2

run_ram:
	./obj_dir/Vm20k_bram_core

# Clean build artifacts
clean:
	rm -rf obj_dir/

help:
	@echo "Available targets:"
	@echo "  ver_transpose - Compile and run the transpose engine rtl/testbench. "
	@echo "  	Set MATRIX_DIM to change matrix size."
	@echo "  ver_ram - Compile and run the m20k bram model rtl/testbench."
	@echo "  	Set LOG_WIDTH/DEPTH to change logical width/depth. The current test bench doesn't support logical widths > 32 bit."
	@echo "  build_transpose - Build the transpose engine executable"
	@echo "  build_ram - Build the m20k bram model executable"
	@echo "  run_transpose - Run the transpose engine executable"
	@echo "  run_ram - Run the m20k bram model executable"
	@echo "  clean - Remove build artifacts"
	@echo "  help - Show this help message"