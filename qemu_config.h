#ifndef __QEMU_CONFIG_H__
#define __QEMU_CONFIG_H__

/*RAM RANGE*/
#define RAM_START   0x40000000
#define RAM_SIZE    0x100000000
#define RAM_END     (RAM_START + RAM_SIZE)

/*Kernel start*/
#define KERNEL_ENTRY_ADDR   RAM_START

/*Stack Size*/
#define STACK_SIZE  0x1000

#endif