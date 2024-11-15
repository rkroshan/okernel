#!/bin/bash
python3 build_ninja.py --target=qemu
ninja -t clean
ninja
qemu-system-aarch64 -machine virt,gic-version=3 -m 4G -smp cpus=4 -cpu max -nographic -kernel build/kernel.elf 
# qemu-system-aarch64 -machine virt,gic-version=3 -m 4G -smp cpus=4 -cpu max -nographic -gdb tcp::5416 -S -kernel build/kernel.elf