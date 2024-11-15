#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "board.h"
#include "errno.h"
#include "thread.h"
/**
 * @brief per physcial cpu structure to hold per core information
 * like which thread is currently running
 *
 */
typedef struct cpu {
  uint64_t cpu_id; /*this is physical local cpu affinity*/
  uint64_t affinity;
  thread_t *current_thread;
  thread_t *idle_thread;
} cpu_t;

/**
 * @brief global kernel structure to hold very physical cpu state
 *
 */
typedef struct kernel {
  /*per cpu state information*/
  cpu_t cpu[MAX_CPUS];
} kernel_t;

/**
 * @brief primary core 0 cold boot init
 *
 */
void primary_boot_cold_init();

/**
 * @brief secondary cold boot init
 *
 */
void secondary_boot_cold_init();

/**
 * @brief set current log level
 *
 */
void set_current_log_level(log_level_e loglevel);

/**
 * @brief set current log level
 *
 */
log_level_e get_current_log_level(void);

#endif