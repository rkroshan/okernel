/* -*- mode: C; coding:utf-8 -*- */
/**********************************************************************/
/*  OS kernel sample                                                  */
/*  Copyright 2014 Takeharu KATO                                      */
/*                                                                    */
/*  AArch64 definitions                                               */
/*                                                                    */
/**********************************************************************/
#ifndef _AARCH64_H
#define _AARCH64_H

#include <stdint.h>
/* CurrentEL, Current Exception Level */
#define CURRENT_EL_MASK 0x3
#define CURRENT_EL_SHIFT 2
/*MPIDR MASR*/
#define MPIDR_AFF0_MASK 0xffU

/* DAIF, Interrupt Mask Bits */
#define DAIF_DBG_BIT (1 << 3) /* Debug mask bit */
#define DAIF_ABT_BIT (1 << 2) /* Asynchronous abort mask bit */
#define DAIF_IRQ_BIT (1 << 1) /* IRQ mask bit */
#define DAIF_FIQ_BIT (1 << 0) /* FIQ mask bit */

/*
 * Interrupt flags
 */
#define AARCH64_DAIF_FIQ (1) /* FIQ */
#define AARCH64_DAIF_IRQ (2) /* IRQ */

/* Timer */
#define CNTV_CTL_ENABLE (1 << 0) /* Enables the timer */
#define CNTV_CTL_IMASK (1 << 1)  /* Timer interrupt mask bit */
#define CNTV_CTL_ISTATUS                                                       \
  (1 << 2) /* The status of the timer interrupt. This bit is read-only */

/* Wait For Interrupt */
#define wfi() asm volatile("wfi" : : : "memory")

/* PSTATE and special purpose register access functions */
uint64_t raw_read_current_el(void);
uint32_t get_current_el(void);
uint64_t raw_read_daif(void);
void raw_write_daif(uint64_t daif);
void enable_debug_exceptions(void);
void enable_serror_exceptions(void);
void enable_irq(void);
void enable_fiq(void);
void disable_debug_exceptions(void);
void disable_serror_exceptions(void);
void disable_irq(void);
void disable_fiq(void);
/* SPSR_EL1, Saved Program Status Register (EL1) */
uint64_t raw_read_spsr_el1(void);
void raw_write_spsr_el1(uint64_t spsr_el1);
/* ISR_EL1, Interrupt Status Register */
uint64_t raw_read_isr_el1(void);
uint64_t raw_read_rvbar_el1(void);
uint64_t raw_read_vbar_el1(void);
void raw_write_vbar_el1(uint64_t vbar_el1);
/*For disabling advance simd instruction trap*/
void enable_fp_simd_access();
/*get mpidr el1 reg value*/
uint64_t get_mpidr();
/*get far el1 reg value*/
uint64_t get_FAR_EL1();
/**
 * @brief set TPIDR_EL1 Value
 */
void set_tpidr_el1(uint64_t tpidr_el1);

#endif /*  _AARCH64_H   */
