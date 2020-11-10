
export ABI   			?=ilp32
export ARCH				?=i
export ITERATIONS	?=700
# Paths
export root_dir := $(shell pwd)
export tst_dir  := $(root_dir)/tests
export inc_dir  := $(tst_dir)/common
export bld_dir  := $(root_dir)/build



# Environment
export CROSS_PREFIX  ?= riscv64-unknown-elf-
export RISCV_GCC     ?= $(CROSS_PREFIX)gcc
export RISCV_OBJDUMP ?= $(CROSS_PREFIX)objdump -D
export RISCV_OBJCOPY ?= $(CROSS_PREFIX)objcopy -O verilog
export RISCV_READELF ?= $(CROSS_PREFIX)readelf -s
#--
coremark: | $(bld_dir)
	-$(MAKE) -C $(tst_dir)/coremark ARCH=$(ARCH)
$(bld_dir):
	mkdir -p $(bld_dir)

.PHONY: clean 	
clean:
	$(RM) -R $(root_dir)/build/*
