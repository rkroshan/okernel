#!/bin/bash
python3 build_ninja.py
ninja -t clean
ninja
qemu-system-aarch64 -machine virt -m 4G -cpu cortex-a57 -nographic -kernel build/kernel.elf