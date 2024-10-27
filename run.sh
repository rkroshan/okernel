#!/bin/bash
python3 build_ninja.py --target=qemu
ninja -t clean
ninja
qemu-system-aarch64 -machine virt,gic-version=3 -m 4G -cpu cortex-a72 -nographic -kernel build/kernel.elf
# qemu-system-aarch64 -machine virt,gic-version=3 -m 4G -cpu cortex-a72 -nographic -gdb tcp::5416 -S -kernel build/kernel.elf