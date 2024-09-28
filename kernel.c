#include <stdint.h>
#include "board.h"
#include "gic_v3.h"
#include "aarch64.h"
#include "uart.h"
#include "timer.h"
#include "atomic.h"

/* Exception SVC Test */
void exception_svc(void)
{
/*
Supervisor call to allow application code to call the OS. 
	It generates an exception targeting exception level 1 (EL1).
*/
	asm("svc #0xdead");
}

/* Main */
void exception_svc_test(void)
{
	uart_puts("exception_svc_test... start\n");
	/* SVC instruction causes a Supervisor Call exception. */ 
	/* vector_table:_curr_el_spx_sync should be called */
	exception_svc();

	// Wait for Interrupt.
	// wfi();
	uart_puts("exception_svc_test... done\n");
}

/*atomic relaxed test*/
void atomic_relaxed_test(void)
{
	_Atomic int atomic_var = 0;

    // atomic_fetch_add(&atomic_var, 1);
	atomic_store_relaxed(&atomic_var,1);
	atomic_load_relaxed(&atomic_var);

    uart_puts("\nAtomic Value: "); uart_puthex((uint64_t)atomic_var);uart_puts("\n"); 
}

/*printk functionality test*/
void print_test()
{
    printk("Hi just checking %d %x %s %u %d\n", 123, 0x123, "0x123", -123, -123);
}

int main(void) {
	enable_fp_simd_access();
	atomic_relaxed_test();
	print_test();
	// uart_puthex(get_current_el()); uart_puts("\n");
	// exception_svc_test();
	timer_test();
}


