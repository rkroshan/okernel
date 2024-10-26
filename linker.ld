#include "qemu_config.h"

EXTERN(_start)
ENTRY(_start_phy)

PHDRS
{
	text PT_LOAD;
	rodata PT_LOAD;
	dynreloc PT_LOAD;
	dynamic PT_DYNAMIC;
	data PT_LOAD;
	tls PT_TLS;
}

SECTIONS
{
	/* Starts at LOADER_ADDR. */
	_start_phy = ABSOLUTE(_start + KERNEL_ENTRY_ADDR);
	. = 0x0;
	.text : AT (KERNEL_ENTRY_ADDR)
	{
		KEEP(*(.text.boot))
		*(SORT_BY_ALIGNMENT(.text .text.*))
		KEEP(*(.text.debug));
	} : text
	. = ALIGN(4096); /* align to page size */

	rodata_base = .;
	.rodata : {
		*(SORT_BY_ALIGNMENT(.rodata .rodata.*))
	} : rodata

	. = ALIGN(8);
	.rela.dyn : { *(.rela.dyn) }

	. = ALIGN(8);
	.gnu.hash : { *(.gnu.hash) }

	. = ALIGN(8);
	.note : { *(.note) *(.note.*) }

	. = ALIGN(4096);
	.dynsym : { *(.dynsym) }
	. = ALIGN(8);
	.dynstr : { *(.dynstr) }

	. = ALIGN(4096);
	.dynamic : { *(.dynamic) } : dynamic : dynreloc
	
	/*Neccessary for enhancing security as NX for data section*/
	. = ALIGN(0x200000);
	.data :
	{
		*(SORT_BY_ALIGNMENT(.data .data.*))
	} : data

	. = ALIGN(8); 
	.bss (NOLOAD) :
	{
		*(SORT_BY_ALIGNMENT(.bss .bss.*))
		*(COMMON)
	}
	. = ALIGN(64); /* align to page size */

	. = . + STACK_SIZE ; /* stack memory */
	stack_top = . + _start_phy;
}
