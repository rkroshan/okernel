#include "spinlock.h"
#include "aarch64.h"
#include "assert.h"
#include "errno.h"
#include "thread.h"
#include "util.h"
#include <stdbool.h>

extern void uart_puts(const char *s);

bool double_spin_check(spinlock_t *lock) {
  uintptr_t cpu = lock->thread_cpu;

  if (cpu != UINT64_MAX) {
    if (cpu == get_current_cpuid()) {
      return true; /*double_spin*/
    }
  }
  return false;
}

/**
 * @brief function to lock spinlock
 */
void spinlock_acquire(spinlock_t *lock) {
  /*disable preemption*/
  disable_irq();
  /*check if the lock is already acquired by current thread*/
  if (double_spin_check(lock)) {
    // uart_puts("double spin lock acquire attempt from same cpu !!\n");
    return;
  }
  /*now inc the tail value*/
  uint64_t ticket =
      atomic_fetch_add_explicit(&lock->tail, 1, memory_order_relaxed);
  /*spin until ticket is not equal to owner*/
  while (atomic_load_acquire(&lock->owner) != ticket) {
    // __asm__ volatile("wfe" ::"m"(lock->owner));
  }

  /*we got the ticket*/
  /*set the current cpu*/
  lock->thread_cpu = get_current_cpuid();
  lock->thread = get_current_thread();
}

/**
 * @brief function to try getting spinlock
 *
 */
uint8_t try_spinlock_acquire(spinlock_t *lock) {
  /*disable preemption*/
  disable_irq();
  /*check if the lock is already acquired by current thread*/
  if (double_spin_check(lock)) {
    // uart_puts("double spin lock acquire attempt from same cpu !!\n");
    return EFAILURE;
  }
  /*now get the owner value*/
  uint64_t ticket = atomic_load_relaxed(&lock->owner);
  /*try to atomic compare and set the tail inc with 1 if tail is equal to
   * ticket*/
  if (!atomic_compare_exchange_strong_explicit(
          &lock->tail, &ticket, ticket + 1U, memory_order_acquire,
          memory_order_relaxed)) {
    goto failed;
  }

  /*we got the ticket*/
  /*set the current cpu*/
  lock->thread_cpu = get_current_cpuid();
  lock->thread = get_current_thread();

  return ESUCCESS;
failed:
  /*reenable the preemption since we failed to acquire*/
  enable_irq();
  return EBUSY;
}

/**
 * @brief function to unlock the spinlock
 *
 */
void spinlock_release(spinlock_t *lock) {
  /*check if spinlock is actually locked*/
  if (atomic_load_relaxed(&lock->owner) == atomic_load_relaxed(&lock->tail)) {
    // uart_puts("attemp to unlock already unlocked spinlock !!\n");
    return;
  }

  /*check if thread and cpu are same*/
  if ((lock->thread_cpu != get_current_cpuid()) ||
      (lock->thread != get_current_thread())) {
    // uart_puts("attempt to unlock spin lock not owned by thread or cpu !!\n");
    return;
  }

  /*increment the owner ticket*/
  uint64_t ticket = atomic_load_relaxed(&lock->owner);
  atomic_store_explicit(&lock->owner, ticket + 1U, memory_order_release);
  /*clear the thread and cpu information*/
  lock->thread = NULL;
  lock->thread_cpu = UINT64_MAX;
  /*enable irq*/
  enable_irq();
}
