#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include "atomic.h"
#include "thread.h"
#include <stdint.h>
/**
 * @brief ticket based spinklock
 *
 */

typedef struct spinlock {
  /*Ticket spinlock
  - ticket
  - owner = Spinlock owner ticket value
  - tail = incremented ticket to be assigned to new acquirer
  - when owner = ticket, spinlock can be acquired, else keep spining until
  owner != ticket
  - when spinlock is released owner value is incremented for the next acquirer
  to acquire
  */
  uint64_t _Atomic owner;
  uint64_t _Atomic tail;

  /*id of cpu which holds the spinlock will be useful to find deadlock*/
  uint64_t thread_cpu;
  /*thread which holds the lock*/
  thread_t *thread;

} spinlock_t;

/**
 * @brief function to init the spinlock
 *
 */

#define DECALRE_SPINLOCK(name)                                                 \
  spinlock_t name = (spinlock_t) {                                             \
    .owner = 0UL, .tail = 0UL, .thread_cpu = UINT64_MAX, .thread = NULL,       \
  }

/**
 * @brief function to lock spinlock
 */
void spinlock_acquire(spinlock_t *lock);

/**
 * @brief function to try getting spinlock
 *
 */
uint8_t try_spinlock_acquire(spinlock_t *lock);

/**
 * @brief function to unlock the spinlock
 *
 */
void spinlock_release(spinlock_t *lock);

#endif