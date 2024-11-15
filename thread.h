#ifndef __THREAD_H__
#define __THREAD_H__

#include <stdint.h>

/**
 * @brief allowed thread states
 *
 */
typedef enum thread_states { READY = 0, RUNNING, BLOCKED } thread_states_t;

/**
 * @brief arm aarch64 cpu registers
 *
 */
typedef struct cpu_registers {
  uint64_t x0;
  uint64_t x1;
  uint64_t x2;
  uint64_t x3;
  uint64_t x4;
  uint64_t x5;
  uint64_t x6;
  uint64_t x7;
  uint64_t x8;
  uint64_t x9;
  uint64_t x10;
  uint64_t x11;
  uint64_t x12;
  uint64_t x13;
  uint64_t x14;
  uint64_t x15;
  uint64_t x16;
  uint64_t x17;
  uint64_t x18;
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t x29;
  uint64_t x30; /*LR*/
  uint64_t esr;
  uint64_t sp;
  uint64_t elr;
  uint64_t spsr;
} cpu_regs_t;

/**
 * @brief reason for blocking this thread from scheduling
 *
 */
#define BLOCKED_REASON_IO ((uint64_t)1 << 0)
#define BLOCKED_REASON_MUTEX ((uint64_t)1 << 1)
#define BLOCKED_REASON_UNBLOCKED ((uint64_t)0)

/**
 * @brief threads priority range
 * lower priority number leads to high precedence
 *
 */
#define THREAD_HIGHEST_PRIORITY (0U)
#define THREAD_LOWEST_PRIORITY (63U)

/**
 * @brief thread struct that will hold information about:
 * - cpu state (registers/ stack pointer/ program counter)
 * - scheduling/blocking reason
 * - its ID
 * when that thread was running before context switched
 * due to exception handling/scheduling
 */
typedef struct thread {
  thread_states_t current_state; /* BLOCKED, RUNNING, READY*/
  uint32_t TID;                  /*thread id*/
  uint32_t priority;             /*thread's priority*/
  uint8_t padding[4];
  uint64_t current_cpuid;  /*current cpuid on which it is runnning*/
  uint64_t blocked_reason; /*why this thread can't be scheduled?*/
  cpu_regs_t registers;
} thread_t;

/**
 * @brief set thread in TPIDR_EL1
 * we will set the tls base in tpidr_el1
 * so that thread_local variable can be referenced
 * from it by the compiler
 */
extern void setup_thread(thread_t *thread);

/**
 * @brief create thread
 *
 */
thread_t *create_thread(void);

/**
 * @brief get current thread
 * compiler will make tpidr_el1(tls_base) + offset(current_thread)
 */
thread_t *get_current_thread(void);

/**
 * @brief Get the current cpuid
 *
 * @return cpuid
 */
uint64_t get_current_cpuid();

#endif