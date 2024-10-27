#ifndef __QEMU_H__
#define __QEMU_H__

/*RAM RANGE*/
#define RAM_START 0x40000000
#define RAM_SIZE 0x100000000
#define RAM_END (RAM_START + RAM_SIZE)

/*Kernel start*/
#define KERNEL_ENTRY_ADDR RAM_START

/*Stack Size*/
#define STACK_SIZE 0x1000

/*For Qemu Virt Aarch64 Arm SOC*/
#define VIRT_UART_ADDR 0x09000000

/*
 * GIC on QEMU Virt
 */
#define QEMU_VIRT_GIC_BASE (0x08000000)
#define QEMU_VIRT_GIC_INT_MAX (1020 - 1)
#define QEMU_VIRT_GIC_PRIO_MAX (16)
/* SGI: Interrupt IDs 0-15 */
/* PPI: Interrupt IDs 16-31 */
/* SPI: Interrupt IDs 32-63 */
#define QEMU_VIRT_GIC_INTNO_SGIO (0)
#define QEMU_VIRT_GIC_INTNO_PPIO (16)
#define QEMU_VIRT_GIC_INTNO_SPIO (32)

#define GIC_BASE (QEMU_VIRT_GIC_BASE)
#define GIC_DIST (GIC_BASE)
#define GIC_REDIST (0x080A0000)
#define GIC_INT_MAX (QEMU_VIRT_GIC_INT_MAX)
#define GIC_PRIO_MAX (QEMU_VIRT_GIC_PRIO_MAX)
#define GIC_INTNO_SGI0 (QEMU_VIRT_GIC_INTNO_SGIO)
#define GIC_INTNO_PPI0 (QEMU_VIRT_GIC_INTNO_PPIO)
#define GIC_INTNO_SPI0 (QEMU_VIRT_GIC_INTNO_SPIO)

#define GIC_PRI_SHIFT (4)
#define GIC_PRI_MASK (0x0f)

#define TIMER_IRQ (27) /** Timer IRQ  */

#endif