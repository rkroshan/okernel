#include "qemu_config.h"

ENTRY(_start)

SECTIONS
{
	/* Starts at LOADER_ADDR. */
	. = KERNEL_ENTRY_ADDR;
	__start = .;
	__text_start = .;
	.text :
	{
	KEEP(*(.text.boot))
	*(.text)
	}
	. = ALIGN(4096); /* align to page size */
	__text_end = .;

	__data_start = .;
	.data :
	{
	*(.data)
	}
	. = ALIGN(4096); /* align to page size */
	__data_end = .;

	__bss_start = .;
	.bss :
	{
	bss = .;
	*(.bss)
	}
	. = ALIGN(4096); /* align to page size */
	__bss_end = .;
	__end = .;

	. = . + STACK_SIZE ; /* stack memory */
	stack_top = .;
}
__bss_size = (__bss_end - __bss_start)>>3;
