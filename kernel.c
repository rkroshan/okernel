#include "kernel.h"
#include "aarch64.h"
#include "assert.h"
#include "atomic.h"
#include "board.h"
#include "gic.h"
#include "idle.h"
#include "mm.h"
#include "psci.h"
#include "timer.h"
#include <stdint.h>

extern void _start(void);

/**
 * @brief kernel structure for smp cpu information
 *
 */

kernel_t kernel;
log_level_e current_log_level;

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
  printk_debug("exception_svc_test... start\n");
  /* SVC instruction causes a Supervisor Call exception. */
  /* vector_table:_curr_el_spx_sync should be called */
  exception_svc();

  // Wait for Interrupt.
  // wfi();
  printk_debug("exception_svc_test... done\n");
}

/**
 * @brief atomic relaxed test
 *
 * test atomic functions working
 * @param None
 * @return
 */
void atomic_test(void) {
  _Atomic int atomic_var = 0;
  uint64_t old_val =
      atomic_fetch_add_explicit(&atomic_var, 4, memory_order_relaxed);
  printk_debug("Atomic_test old:%u new:%u\n", old_val, atomic_var);
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
  printk_debug("CurrentEL = %x\n", raw_read_current_el());
  printk_debug("RVBAR_EL1 = %x\n", raw_read_rvbar_el1());
  printk_debug("VBAR_EL1 = %x\n", raw_read_vbar_el1());
  printk_debug("DAIF = %x\n", raw_read_daif());
  printk_debug("Hi just checking %d %x %s %u %d %b\n", 123, 4096, "0x123", -123,
               -123, 1024);
}

/**
 * @brief primary core 0 cold boot init
 * Main function to setup initalize the system after _start
 *
 * enable floating pointer simd access
 * test atomic functions working
 * test fomatiing prink function working
 * commented : test spx elx sync exception testing
 */

void primary_boot_cold_init() {
  // enable floating pointer simd access
  enable_fp_simd_access();

  // set current log level
  set_current_log_level(INFO);

  /*setup the heap management*/
  boot_mem_init();

  /*set the current cpu information*/
  uint64_t affinity = get_mpidr();
  uint64_t cpu_id = affinity & MPIDR_AFF0_MASK;
  cpu_t *_current_cpu = &kernel.cpu[cpu_id];
  _current_cpu->cpu_id = cpu_id;
  _current_cpu->affinity = affinity;
  /*setup the idle thread*/
  idle_thread_init(_current_cpu->cpu_id);
  _current_cpu->idle_thread = get_current_thread();
  _current_cpu->current_thread = _current_cpu->idle_thread;

  /*do peripheral initialisation*/
  // test atomic functions working
  atomic_test();
  // test formating prink function working
  print_test();

  // GIC Init
  primary_init_interrupt_controller();

  // Platoform timer init
  platform_timer_init();

  /*put the secondary core out of reset*/
  for (uint8_t id = 1; id < MAX_CPUS; id++) {
    psci_cpu_on(id, (uint64_t)_start);
  }
  /*call idle thread*/
  idle();
}

/**
 * @brief secondary cold boot init
 *
 */
void secondary_boot_cold_init() {
  /*set the current cpu information*/
  uint64_t affinity = get_mpidr();
  uint64_t cpu_id = affinity & MPIDR_AFF0_MASK;
  cpu_t *_current_cpu = &kernel.cpu[cpu_id];
  _current_cpu->cpu_id = cpu_id;
  _current_cpu->affinity = affinity;
  /*setup the idle thread*/
  idle_thread_init(_current_cpu->cpu_id);
  _current_cpu->idle_thread = get_current_thread();
  _current_cpu->current_thread = _current_cpu->idle_thread;

  /*do peripheral initialisation*/
  // enable floating pointer simd access
  enable_fp_simd_access();
  // test atomic functions working
  atomic_test();
  // test formating prink function working
  print_test();

  // GIC Init
  secondary_init_interrupt_controller();

  // Platoform timer init
  platform_timer_init();

  /*call idle thread*/
  idle();
}

/**
 * @brief set current log level
 *
 */
void set_current_log_level(log_level_e loglevel) {
  current_log_level = loglevel;
}

/**
 * @brief set current log level
 *
 */
log_level_e get_current_log_level(void) { return current_log_level; }
