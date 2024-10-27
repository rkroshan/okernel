#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdbool.h>
#include <stdint.h>

void platform_timer_init(void);
void platform_timer_handler(void);

uint64_t get_current_ticks();
void platform_timer_set_timeout_in_sec(uint64_t timeout);
uint64_t raw_read_cntv_cval_el0(void);
uint64_t raw_read_cntfrq_el0(void);
void platform_timer_mask_interrupt(bool state);
uint64_t raw_read_cntv_ctl_reg(void);

void platform_timer_enable(bool state);

#endif /* __TIMER_H__  */
