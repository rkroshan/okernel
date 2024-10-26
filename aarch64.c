/*
 * Reference: ARM® Architecture Reference Manual ARMv8, for ARMv8-A architecture
 * profile
 */
#include "aarch64.h"
#include "atomic.h"
#include <stdint.h>

/* CurrentEL, Current Exception Level
        EL, bits [3:2]
                Current exception level. Possible values of this field are:
                00 EL0
                01 EL1
                10 EL2
                11 EL3
*/
uint64_t raw_read_current_el(void) {
  uint64_t current_el;

  __asm__ __volatile__("mrs %0, CurrentEL\n\t" : "=r"(current_el) : : "memory");
  return current_el;
}

uint32_t get_current_el(void) {
  uint32_t current_el = raw_read_current_el();
  return ((current_el >> CURRENT_EL_SHIFT) & CURRENT_EL_MASK);
}

/*Disable Advance SIMD instruction to get trapped*/
void enable_fp_simd_access() {
  unsigned long cpacr_el1;

  // Read CPACR_EL1 register
  __asm__ volatile("mrs %0, CPACR_EL1" : "=r"(cpacr_el1));

  // Set bits [21:20] to '11' to allow full access to SIMD/FP
  cpacr_el1 |= (0x3 << 20);

  // Write back to CPACR_EL1
  __asm__ volatile("msr CPACR_EL1, %0" : : "r"(cpacr_el1));

  // setup the instruction since we are not context switiching
  instruction_barrier();
}

/**
 * @brief get MPIDR_EL1 Value
 * @return mpidr_el1[39:0]
 */
uint64_t get_mpidr() {
  uint64_t mpidr_el1;
  // READ MPIDR_EL1 register
  __asm__ volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr_el1));
  mpidr_el1 = mpidr_el1 & 0xff00ffffff;
  return mpidr_el1;
}

/**
 * @brief Get the FAR EL1
 *
 * @return uint64_t
 */
uint64_t get_FAR_EL1() {
  uint64_t far_el1;
  // READ FAR_EL1 register
  __asm__ volatile("mrs %0, FAR_EL1" : "=r"(far_el1));
  return far_el1;
}

/* DAIF, Interrupt Mask Bits
        Allows access to the interrupt mask bits.

        D, bit [9]: Debug exceptions.
        A, bit [8]: SError (System Error) mask bit.
        I, bit [7]: IRQ mask bit.
        F, bit [6]: FIQ mask bit.
        value:
                0 Exception not masked.
                1 Exception masked.
*/
uint64_t raw_read_daif(void) {
  uint64_t daif;

  __asm__ __volatile__("mrs %0, DAIF\n\t" : "=r"(daif) : : "memory");
  return daif;
}

void raw_write_daif(uint64_t daif) {
  __asm__ __volatile__("msr DAIF, %0\n\t" : : "r"(daif) : "memory");
}

void enable_debug_exceptions(void) {
  __asm__ __volatile__("msr DAIFClr, %0\n\t" : : "i"(DAIF_DBG_BIT) : "memory");
}

void enable_serror_exceptions(void) {
  __asm__ __volatile__("msr DAIFClr, %0\n\t" : : "i"(DAIF_ABT_BIT) : "memory");
}

void enable_irq(void) {
  __asm__ __volatile__("msr DAIFClr, %0\n\t" : : "i"(DAIF_IRQ_BIT) : "memory");
}

void enable_fiq(void) {
  __asm__ __volatile__("msr DAIFClr, %0\n\t" : : "i"(DAIF_FIQ_BIT) : "memory");
}

void disable_debug_exceptions(void) {
  __asm__ __volatile__("msr DAIFSet, %0\n\t" : : "i"(DAIF_DBG_BIT) : "memory");
}

void disable_serror_exceptions(void) {
  __asm__ __volatile__("msr DAIFSet, %0\n\t" : : "i"(DAIF_ABT_BIT) : "memory");
}

void disable_irq(void) {
  __asm__ __volatile__("msr DAIFSet, %0\n\t" : : "i"(DAIF_IRQ_BIT) : "memory");
}

void disable_fiq(void) {
  __asm__ __volatile__("msr DAIFSet, %0\n\t" : : "i"(DAIF_FIQ_BIT) : "memory");
}

/* SPSR_EL1, Saved Program Status Register (EL1)
        Holds the saved processor state when an exception is taken to EL1.
*/
uint64_t raw_read_spsr_el1(void) {
  uint64_t spsr_el1;

  __asm__ __volatile__("mrs %0, SPSR_EL1\n\t" : "=r"(spsr_el1) : : "memory");
  return spsr_el1;
}

void raw_write_spsr_el1(uint64_t spsr_el1) {
  __asm__ __volatile__("msr SPSR_EL1, %0\n\t" : : "r"(spsr_el1) : "memory");
}

/*
ISR_EL1, Interrupt Status Register
        Shows whether an IRQ, FIQ, or SError interrupt is pending.
*/
uint64_t raw_read_isr_el1(void) {
  uint64_t isr_el1;

  __asm__ __volatile__("mrs %0, ISR_EL1\n\t" : "=r"(isr_el1) : : "memory");
  return isr_el1;
}

/*
RVBAR_EL1, Reset Vector Base Address Register (if EL2 and EL3 not implemented)
        If EL1 is the highest exception level implemented, contains the
        IMPLEMENTATION DEFINED address that execution starts from after reset
when executing in AArch64 state.
*/
uint64_t raw_read_rvbar_el1(void) {
  uint64_t rvbar_el1;

  __asm__ __volatile__("mrs %0, RVBAR_EL1\n\t" : "=r"(rvbar_el1) : : "memory");
  return rvbar_el1;
}

/* VBAR_EL1, Vector Base Address Register (EL1)
        Holds the exception base address for any exception that is taken to EL1.
*/
uint64_t raw_read_vbar_el1(void) {
  uint64_t vbar_el1;

  __asm__ __volatile__("mrs %0, VBAR_EL1\n\t" : "=r"(vbar_el1) : : "memory");
  return vbar_el1;
}

void raw_write_vbar_el1(uint64_t vbar_el1) {
  __asm__ __volatile__("msr VBAR_EL1, %0\n\t" : : "r"(vbar_el1) : "memory");
}

/* CNTV_CTL_EL0, Counter-timer Virtual Timer Control register
        Control register for the virtual timer.

        ISTATUS, bit [2]:	The status of the timer interrupt.
        IMASK, bit [1]:		Timer interrupt mask bit.
        ENABLE, bit [0]:	Enables the timer.
*/
uint64_t raw_read_cntv_ctl(void) {
  uint64_t cntv_ctl;

  __asm__ __volatile__("mrs %0, CNTV_CTL_EL0\n\t"
                       : "=r"(cntv_ctl)
                       :
                       : "memory");
  return cntv_ctl;
}

void disable_cntv(void) {
  uint64_t cntv_ctl;

  cntv_ctl = raw_read_cntv_ctl();
  cntv_ctl &= ~CNTV_CTL_ENABLE;
  __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t" : : "r"(cntv_ctl) : "memory");
}

void enable_cntv(void) {
  uint64_t cntv_ctl;

  cntv_ctl = raw_read_cntv_ctl();
  cntv_ctl |= CNTV_CTL_ENABLE;
  __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t" : : "r"(cntv_ctl) : "memory");
}

/*
CNTFRQ_EL0, Counter-timer Frequency register
        Holds the clock frequency of the system counter.
*/
uint64_t raw_read_cntfrq_el0(void) {
  uint64_t cntfrq_el0;

  __asm__ __volatile__("mrs %0, CNTFRQ_EL0\n\t"
                       : "=r"(cntfrq_el0)
                       :
                       : "memory");
  return cntfrq_el0;
}

void raw_write_cntfrq_el0(uint64_t cntfrq_el0) {
  __asm__ __volatile__("msr CNTFRQ_EL0, %0\n\t" : : "r"(cntfrq_el0) : "memory");
}

/* CNTVCT_EL0, Counter-timer Virtual Count register
        Holds the 64-bit virtual count value.
*/
uint64_t raw_read_cntvct_el0(void) {
  uint64_t cntvct_el0;

  __asm__ __volatile__("mrs %0, CNTVCT_EL0\n\t"
                       : "=r"(cntvct_el0)
                       :
                       : "memory");
  return cntvct_el0;
}

/* CNTV_CVAL_EL0, Counter-timer Virtual Timer CompareValue register
        Holds the compare value for the virtual timer.
*/
uint64_t raw_read_cntv_cval_el0(void) {
  uint64_t cntv_cval_el0;

  __asm__ __volatile__("mrs %0, CNTV_CVAL_EL0\n\t"
                       : "=r"(cntv_cval_el0)
                       :
                       : "memory");
  return cntv_cval_el0;
}

void raw_write_cntv_cval_el0(uint64_t cntv_cval_el0) {
  __asm__ __volatile__("msr CNTV_CVAL_EL0, %0\n\t"
                       :
                       : "r"(cntv_cval_el0)
                       : "memory");
}
