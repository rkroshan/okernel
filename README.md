## ninja build
*	Prerequisite
	```
	sudo apt install -y ninja-build qemu-system-aarch64 doxygen doxygen-doc doxygen-gui doxygen-latex clang-format llvm clang gdb-multiarch

	```
	```
	python3 -m pip install ninja_syntax autopep8
	```
*	Build
	```
	python3 build_ninja.py
	```
*	Run
	```
	./run.sh
	```

# armv8-bare-metal
*	Purpose
	* It's a bare-metal study in QEMU (-M virt -cpu max)
	* _start -> main()
*	How to run
	```
	./run.sh
	```
*	GDB (Terminal 1/2 should be in the same directory.)
	```
	Terminal 1:
		qemu-system-aarch64 -machine virt,gic-version=3 -m 4G -smp cpus=4 -cpu max -nographic -gdb tcp::5416 -S -kernel build/kernel.elf
	Terminal 2:
		gdb-multiarch -x gdx
	```
*	Timer IRQ works. It assert Timer_Handler() every 1 sec.
	```
	[1.145950]CPU3 [INFO] platform_timer_handler: irq: 1Bh
	[1.146521]CPU0 [INFO] platform_timer_handler: irq: 1Bh
	[1.146642]CPU2 [INFO] platform_timer_handler: irq: 1Bh
	[1.146745]CPU1 [INFO] platform_timer_handler: irq: 1Bh
	[1.146841]CPU3 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[1.146985]CPU0 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[1.147144]CPU2 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[1.147247]CPU1 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[1.147344]CPU3 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[1.147429]CPU0 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[1.147549]CPU2 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[1.147643]CPU1 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[2.147265]CPU3 [INFO] platform_timer_handler: irq: 1Bh
	[2.147425]CPU2 [INFO] platform_timer_handler: irq: 1Bh
	[2.147539]CPU3 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[2.147637]CPU0 [INFO] platform_timer_handler: irq: 1Bh
	[2.147733]CPU1 [INFO] platform_timer_handler: irq: 1Bh
	[2.147832]CPU2 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[2.147949]CPU3 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[2.148041]CPU0 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[2.148129]CPU1 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[2.148221]CPU2 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[2.148329]CPU0 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[2.148411]CPU1 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[3.148702]CPU3 [INFO] platform_timer_handler: irq: 1Bh
	[3.148807]CPU3 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[3.148959]CPU1 [INFO] platform_timer_handler: irq: 1Bh
	[3.149060]CPU3 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[3.149128]CPU2 [INFO] platform_timer_handler: irq: 1Bh
	[3.149233]CPU0 [INFO] platform_timer_handler: irq: 1Bh
	[3.149319]CPU1 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[3.149430]CPU2 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[3.149544]CPU0 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[3.149627]CPU1 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[3.149720]CPU2 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[3.149828]CPU0 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[4.150009]CPU0 [INFO] platform_timer_handler: irq: 1Bh
	[4.150105]CPU3 [INFO] platform_timer_handler: irq: 1Bh
	[4.150172]CPU1 [INFO] platform_timer_handler: irq: 1Bh
	[4.150281]CPU0 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[4.150380]CPU3 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[4.150497]CPU1 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[4.150615]CPU2 [INFO] platform_timer_handler: irq: 1Bh
	[4.150708]CPU0 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[4.150800]CPU3 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[4.150888]CPU1 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[4.150997]CPU2 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[4.151087]CPU2 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[5.151265]CPU0 [INFO] platform_timer_handler: irq: 1Bh
	[5.151384]CPU3 [INFO] platform_timer_handler: irq: 1Bh
	[5.151481]CPU1 [INFO] platform_timer_handler: irq: 1Bh
	[5.151567]CPU0 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[5.151687]CPU3 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[5.151795]CPU2 [INFO] platform_timer_handler: irq: 1Bh
	[5.151875]CPU1 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[5.151966]CPU0 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[5.152077]CPU3 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[5.152149]CPU2 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[5.152229]CPU1 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[5.152314]CPU2 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[6.152749]CPU0 [INFO] platform_timer_handler: irq: 1Bh
	[6.152862]CPU0 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[6.152958]CPU2 [INFO] platform_timer_handler: irq: 1Bh
	[6.153060]CPU3 [INFO] platform_timer_handler: irq: 1Bh
	[6.153161]CPU1 [INFO] platform_timer_handler: irq: 1Bh
	[6.153273]CPU0 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[6.153354]CPU2 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[6.153463]CPU3 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[6.153565]CPU1 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	[6.153680]CPU2 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[6.153758]CPU3 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[6.153857]CPU1 [INFO] Enable the timer, CNTV_CTL_EL0 = 1h
	[7.154020]CPU0 [INFO] platform_timer_handler: irq: 1Bh
	[7.154191]CPU0 [INFO] System Frequency: CNTFRQ_EL0 = 62500000
	```

# Reference
*	Project
	*   [armv8-bare-metal](https://github.com/NienfengYao/armv8-bare-metal)
	*	Application Note Bare-metal Boot Code for ARMv8-A Processors Version 1.0
	*	ARM® Architecture Reference Manual ARMv8, for ARMv8-A architecture profile Beta
	*	ARM® Cortex®-A Series Version: 1.0 Programmer’s Guide for ARMv8-A
	*	ARM® Generic Interrupt Controller Architecture Specification GIC architecture version 3.0 and version 4.0
