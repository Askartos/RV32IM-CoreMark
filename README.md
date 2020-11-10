# RV32IM-CoreMark
Compiling setup to run Coremark on baremetal riscv cores
In order to port this tamplate to your platform you have to modify
1) ./test/common/sc_print.c :
	here you have to define any output port for your core e.g. UART
2) ./test/common/ram.lds :
	an example of a linker script so you can modify to adjust it to your platform memory map
3) ./test/common/init_asm.S.lds :
	start function that setup the enviorment to call main clears .bss section sets the stack pointer.
	just modify it if you want something special for your platform.
4) ./test/benchmarks/coremark/core_portme.c :
	Here you should modify timing functions => barebones_clock() start_time() stop_time() start_insn() stop_insn() insn()
	and some enviorment function => portable_init() portable_fini()

runing example :
	make ARCH=im ITERATIONS=300


This repo its based on:
		https://github.com/eembc/coremark
		https://github.com/syntacore/scr1
