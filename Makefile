# Verilog source files
VERILOG_SOURCES = ./rtl/baseline/circulant_barrel_shifter_v2.v ./rtl/common/bram_mem.v 

# C++ testbench
CPP_TESTBENCH = ./tb/tb_with_mem_modules.cpp

# Uses verilator to compile HDL design and c++ testbench into object files
run_verilator: 
	verilator -cc $(VERILOG_SOURCES) --exe $(CPP_TESTBENCH)

# Use make to build an executable from the generated files
build:
	make -C ./obj_dir/ -f Vcirculant_barrel_shifter_v2.mk Vcirculant_barrel_shifter_v2

# Run the executable
run:
	./obj_dir/Vcirculant_barrel_shifter_v2

# Clean build artifacts
clean:
	rm -rf obj_dir/