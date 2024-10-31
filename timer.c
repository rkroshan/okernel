#include "timer.h"
#include "aarch64.h"
#include "assert.h"
#include "board.h"
#include "gic.h"
#include "uart.h"

#define TIME_IN_NSEC (1000000000)
#define TIMESPEC_MAX_NSEC (TIME_IN_NSEC - 1)
static uint64_t cntfrq; /* System frequency */
static uint64_t max_timeout;

/**
 * @brief CNTVCT_EL0, Counter-timer Virtual Count register
 *      Holds the 64-bit virtual count value.
 *
 * @return uint64_t vitual timer current ticks
 */
uint64_t get_current_ticks() {
  uint64_t cntvct_el0;

  __asm__ __volatile__("mrs %0, CNTVCT_EL0\n\t"
                       : "=r"(cntvct_el0)
                       :
                       : "memory");
  return cntvct_el0;
}

/**
 * @brief CNTV_CTL_EL0, Counter-timer Virtual Timer Control register
        Control register for the virtual timer.

        ISTATUS, bit [2]:	The status of the timer interrupt.
        IMASK, bit [1]:		Timer interrupt mask bit.
        ENABLE, bit [0]:	Enables the timer.
 *
 * @return cntv_ctl_el0 reg val
 */
uint64_t raw_read_cntv_ctl_reg(void) {
  uint64_t cntv_ctl;

  __asm__ __volatile__("mrs %0, CNTV_CTL_EL0\n\t"
                       : "=r"(cntv_ctl)
                       :
                       : "memory");
  return cntv_ctl;
}

/**
 * @brief Enable/disable the Virtual timer EL1
 *
 */
void platform_timer_enable(bool state) {
  uint64_t cntv_ctl;

  cntv_ctl = raw_read_cntv_ctl_reg();
  if (state) {
    cntv_ctl |= (state << 0);
  } else {
    cntv_ctl &= ~(state << 0);
  }
  __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t" : : "r"(cntv_ctl) : "memory");
}

/**
 * @brief Enable/disable the Virtual timer EL1 interrupt
 *
 */
void platform_timer_mask_interrupt(bool state) {
  uint64_t cntv_ctl;

  cntv_ctl = raw_read_cntv_ctl_reg();
  if (state) {
    cntv_ctl |= (state << 1);
  } else {
    cntv_ctl &= ~(state << 1);
  }
  __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t" : : "r"(cntv_ctl) : "memory");
}

/**
 * @brief CNTFRQ_EL0, Counter-timer Frequency register
 *      Holds the clock frequency of the system counter.
 *
 * @return uint64_t frequency
 */
uint64_t raw_read_cntfrq_el0(void) {
  uint64_t cntfrq_el0;

  __asm__ __volatile__("mrs %0, CNTFRQ_EL0\n\t"
                       : "=r"(cntfrq_el0)
                       :
                       : "memory");
  return cntfrq_el0;
}

/**
 * @brief CNTV_CVAL_EL0, Counter-timer Virtual Timer CompareValue register
 *      Holds the compare value for the virtual timer.
 *
 * @return uint64_t value of ticks at which next timer should hit
 */
uint64_t raw_read_cntv_cval_el0(void) {
  uint64_t cntv_cval_el0;

  __asm__ __volatile__("mrs %0, CNTV_CVAL_EL0\n\t"
                       : "=r"(cntv_cval_el0)
                       :
                       : "memory");
  return cntv_cval_el0;
}

/**
 * @brief CNTV_CVAL_EL0, Counter-timer Virtual Timer CompareValue register
 *      Holds the compare value for the virtual timer.
 *
 * @param uint64_t counter value at which next tick occurs
 */
static void raw_write_cntv_cval_el0(uint64_t cntv_cval_el0) {
  __asm__ __volatile__("msr CNTV_CVAL_EL0, %0\n\t"
                       :
                       : "r"(cntv_cval_el0)
                       : "memory");
}

/**
 * @brief set the timeout in virtual timer cval reg for next interrupt to occur
 *
 * @param timeout
 */
void platform_timer_set_timeout_in_sec(uint64_t timeout) {
  if (timeout > max_timeout) {
    printk("FAILED TO SETUP TIMEOUT %u > maxtimeout (%u)\n", timeout,
           max_timeout);
  }
  /*convert the timeout seconds into ticks and it to current time*/
  raw_write_cntv_cval_el0(get_current_ticks() + (timeout * cntfrq));
}

/**
 * @brief platform timer isr handler
 *
 */
static void platform_timer_handler(irq_t irq, void *data) {
  (void)data;
  printk("platform_timer_handler: irq: %x\n", irq);

  // Disable the timer
  platform_timer_enable(false);
  gic_clear_pending(TIMER_IRQ);
  printk("System Frequency: CNTFRQ_EL0 = %u\n", cntfrq);

  // set the timer irq
  platform_timer_set_timeout_in_sec(PLATFORM_TIMER_INTERRUPT_INTERVAL);

  // Enable the timer
  platform_timer_mask_interrupt(false);
  platform_timer_enable(true);
  printk("Enable the timer, CNTV_CTL_EL0 = %x\n", raw_read_cntv_ctl_reg());
}

/**
 * @brief intialise platform timer
 *
 */
void platform_timer_init(void) {
  printk("platform_timer_init\n");
  /*read the system counter frequency*/
  cntfrq = raw_read_cntfrq_el0();
  printk("System Frequency: CNTFRQ_EL0 = %u\n", cntfrq);
  assert(cntfrq < UINT64_MAX);

  printk("CNTV_CTL_EL0: %x\n", raw_read_cntv_ctl_reg());
  /*Disable the EL1 virtual timer
  virtual timer is good to use at el1, since
  it is equal to EL1 physical timer, when EL2 is not there
  or if CNTV Offset is 0 even if EL2 is there
  */
  platform_timer_enable(false);

  /*need to calculate max no of seconds we can timeout for
  else system can't hold timer longer than that since uint64 will get overflowed
  also ensures that we’re below the maximum counter value in nanoseconds,
  avoiding overflow.*/
  max_timeout = (UINT64_MAX - TIMESPEC_MAX_NSEC) / cntfrq;

  // set the timer irq inetrrupt interval
  platform_timer_set_timeout_in_sec(PLATFORM_TIMER_INTERRUPT_INTERVAL);

  /*register the platform timer isr*/
  register_interrupt_isr(TIMER_IRQ, &platform_timer_handler, NULL);
  // Enable the timer and unmask the interruot
  platform_timer_mask_interrupt(false);
  platform_timer_enable(true);
  printk("Enable the timer, CNTV_CTL_EL0 = %x\n", raw_read_cntv_ctl_reg());

  // Enable IRQ
  enable_irq();
  printk("Enable IRQ, DAIF = %x\n", raw_read_daif());
}
