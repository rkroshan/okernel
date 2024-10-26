#include "aarch64.h"
#include "assert.h"
#include "atomic.h"
#include "board.h"
#include "gic_v3.h"
#include "timer.h"
#include "uart.h"
#include <stdint.h>

/* Exception SVC Test */
void exception_svc(void) {
  /*
  Supervisor call to allow application code to call the OS.
          It generates an exception targeting exception level 1 (EL1).
  */
  asm("svc #0xdead");
}

/**
 * @brief assert test
 *
 */
void assert_test(void) { assert(123 == 9); }

/**
 * @brief est spx elx sync exception
 *
 * test spx elx sync exception testing
 * @param None
 * @return 0 on success, negative on failure.
 */
void exception_svc_test(void) {
  uart_puts("exception_svc_test... start\n");
  /* SVC instruction causes a Supervisor Call exception. */
  /* vector_table:_curr_el_spx_sync should be called */
  exception_svc();

  // Wait for Interrupt.
  // wfi();
  uart_puts("exception_svc_test... done\n");
}

/**
 * @brief atomic relaxed test
 *
 * test atomic functions working
 * @param None
 * @return
 */
void atomic_relaxed_test(void) {
  _Atomic int atomic_var = 0;

  // atomic_fetch_add(&atomic_var, 1);
  atomic_store_relaxed(&atomic_var, 1);
  atomic_load_relaxed(&atomic_var);

  uart_puts("\nAtomic Value: ");
  uart_puthex((uint64_t)atomic_var);
  uart_puts("\n");
}

/**
 * @brief printk functionality test
 *
 * test fomatiing prink function working
 * @param None
 * @return
 */
/*printk functionality test*/
void print_test() {
  printk("Hi just checking %d %x %s %u %d %b\n", 123, 4096, "0x123", -123, -123,
         1024);
}

/**
 * @brief Main function to setup initalize the system after _start
 *
 * enable floating pointer simd access
 * test atomic functions working
 * test fomatiing prink function working
 * commented : test spx elx sync exception testing
 * @param None
 * @return 0 on success, negative on failure.
 */
int main(void) {
  enable_fp_simd_access();
  atomic_relaxed_test();
  // assert_test();
  print_test();
  // exception_svc_test();
  timer_test();
  return 0;
}
