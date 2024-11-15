#include "thread.h"
#include "aarch64.h"
#include "board.h"
#include "mm.h"
#include "util.h"

extern thread_t _Thread_local current_thread;

thread_t _Thread_local current_thread
    __attribute__((section(".tbss.current_thread")));

extern uint64_t tbss_size;
extern uint64_t tbss_align;

#define _tbss_size (((uint64_t)&tbss_size) - RAM_START)
#define _tbss_align (((uint64_t)&tbss_align) - RAM_START)

/**
 * @brief create thread
 *
 */
thread_t *create_thread(void) {
  /*allocate memory for thread*/
  thread_t *thread = (thread_t *)kmalloc_aligned(_tbss_size, _tbss_align);
  /*clean the memory*/
  memset((void *)thread, 0x0, _tbss_size);
  return thread;
}

/**
 * @brief get current thread
 * compiler will make tpidr_el1 + offset(current_thread)
 */
thread_t *get_current_thread() { return &current_thread; }

/**
 * @brief Get the current cpuid
 *
 * @return cpuid
 */
uint64_t get_current_cpuid() { return (&current_thread)->current_cpuid; }
