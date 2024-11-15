.section .text.vectors

#include "qemu.h"

#define ASM_FILE   1

/*
* @brief Entry Point for EL1 Kernel
*
* This function  only allows cpu0 to follow others put to halt state since currently it doesn't support smp
* then sets up the vbar_el1 , then setup the stack pointer.
* then jumps to main function.
*
*/
.global _start
_start:
	/* check CPU ID = 0x0, or jump to hang */
	mrs	x0, mpidr_el1
	and	x0, x0, #3 
	cmp	x0, #0
	bne secondary_cpu

core0_init:
	adrp	x1, vectors		//set the vbar_el1
	msr	vbar_el1, x1
	adrp	x0, stack_top	// Address of 4KB page at a PC-relative offset
	mov	sp, x0				// sp = stack_top (align with 4KB page)
	bl	primary_boot_cold_init
	b hang					// should not happen, expectation is idle thread always running atleast

secondary_cpu:
secondary_core_init:	//X0 has the mpidr aff0 val
	adrp	x1, vectors		//set the vbar_el1
	msr	vbar_el1, x1
	adrp x1, stack_top
	mov  x2, #STACK_SIZE
	mul	 x3, x2, x0
	sub	x1, x1, x3	//gives the stack top
	mov sp, x1
	bl secondary_boot_cold_init	//jumps to secondary_boot_cold_init
	b hang

hang:
	wfi						// wait for wfi interrupt
	b	hang

.section .data
.align 4
.global SECONDARY_CORE_FLAG
SECONDARY_CORE_FLAG:	.word 0

