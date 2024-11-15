#include "idle.h"

/**
 * @brief idle thread init function
 *
 */
void idle_thread_init(uint64_t cpuid) {
  thread_t *idle_thread = create_thread();
  idle_thread->TID = 0x0;
  idle_thread->current_cpuid = cpuid;
  idle_thread->current_state = READY;
  idle_thread->priority = THREAD_LOWEST_PRIORITY;
  idle_thread->blocked_reason = BLOCKED_REASON_UNBLOCKED;

  /*No needs to setup initial cpu context
  since the first thread that runs is an ideal thread
  so on context switch current cpu regs will get store appropriately*/

  /*setup the thread in tpidr_el1*/
  setup_thread(idle_thread);
}

/**
 * @brief idle thread function
 *
 */
void idle() {
  while (1) {
    // never returns
  }
}
