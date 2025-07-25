# Conditionally build the parameter string
MATRIX_PARAM = $(if $(MATRIX_DIM),--GMATRIX_DIM=$(MATRIX_DIM),)


#LOGICAL_DATA_WIDTH = 8
#LOGICAL_DEPTH = 2048

# Verilog source files
VERILOG_SOURCES = ./rtl/baseline/circulant_barrel_shifter_v2.v $(MATRIX_PARAM) ./rtl/common/bram_mem.v

RAM_MODEL_SOURCES = ./rtl/baseline/m20k_bram_core.v
RAM_MODEL_TESTBENCH = ./tb/tb_m20k.cpp

# C++ testbench
CPP_TESTBENCH = ./tb/tb_with_mem_modules.cpp

# Uses verilator to compile HDL design and c++ testbench into object files
run_verilator: 
	@echo "Compiling with$(if $(MATRIX_DIM), MATRIX_DIM=$(MATRIX_DIM), default MATRIX_DIM)"
	verilator -cc $(VERILOG_SOURCES) --exe $(CPP_TESTBENCH)

run_ver_ram:
	#TODO: add support for compiling diff logical width/depth
	verilator -cc $(RAM_MODEL_SOURCES) --exe $(RAM_MODEL_TESTBENCH)

# Use make to build an executable from the generated files
build:
	make -C ./obj_dir/ -f Vcirculant_barrel_shifter_v2.mk Vcirculant_barrel_shifter_v2

build_ram:
	make -C ./obj_dir/ -f Vm20k_bram_core.mk Vm20k_bram_core

# Run the executable
run:
	./obj_dir/Vcirculant_barrel_shifter_v2

run_ram:
	./obj_dir/Vm20k_bram_core

# Clean build artifacts
clean:
	rm -rf obj_dir/