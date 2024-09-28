.section .boot_text

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
	bne	hang

/*
* @brief Reset Vector Base Address,
*/
reset_vector_base:
	adrp	x1, vectors
	msr	vbar_el1, x1

boot_cpu:
/* 
* @brief configure stack 
*/
	adrp	x0, stack_top	// Address of 4KB page at a PC-relative offset

main_label:					
	mov	sp, x0				// sp = stack_top (align with 4KB page)
	bl	main

hang:
	wfi						// wait for wfi interrupt
	b	hang

