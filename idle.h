#ifndef __IDLE_H__
#define __IDLE_H__

#include "thread.h"
#include <stdint.h>

/**
 * @brief idle thread init function
 *
 * @param cpuid
 * @return thread_t*
 */
void idle_thread_init(uint64_t cpuid);

/**
 * @brief idle thread function
 *
 */
void idle();

#endif