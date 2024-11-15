/* -*- mode: c; coding:utf-8 -*- */
/**********************************************************************/
/*  OS kernel sample                                                  */
/*  Copyright 2014 Takeharu KATO                                      */
/*                                                                    */
/*  Exception handler                                                 */
/*                                                                    */
/**********************************************************************/

#include "exception.h"
#include "aarch64.h"
#include "board.h"
#include "gic.h"
#include "psw.h"

extern void platform_timer_handler(void);

void handle_exception(exception_frame *exc) {
  printk_critical(
      "EXCEPTION: \nexc_type: %x ESR: %x ELR: %x\nSP: %x SPSR: %x FAR_EL1: "
      "%x\nX0: %x X1: %x X2: %x\nX3: %x X4: %x X5: %x\nX6: %x X7: %x X8: "
      "%x\nX9: %x X10: %x X11: %x\nX12: %x X13: %x X14: %x\nX15: %x X16: %x "
      "X17: %x\nX18: %x X19: %x X20: %x\nX21: %x X22: %x X23: %x\nX24: %x X25: "
      "%x X26: %x\nX27: %x X28: %x X29: %x\nX30: %x\n",
      exc->exc_type, exc->exc_esr, exc->exc_elr, exc->exc_sp, exc->exc_spsr,
      get_FAR_EL1(), exc->x0, exc->x1, exc->x2, exc->x3, exc->x4, exc->x5,
      exc->x6, exc->x7, exc->x8, exc->x9, exc->x10, exc->x11, exc->x12,
      exc->x13, exc->x14, exc->x15, exc->x16, exc->x17, exc->x18, exc->x19,
      exc->x20, exc->x21, exc->x22, exc->x23, exc->x24, exc->x25, exc->x26,
      exc->x27, exc->x28, exc->x29, exc->x30);
}

void trigger_isr(irq_t irq) {
  isr_struct_t isr;
  uint8_t ret = get_registered_isr(irq, &isr);
  if (ret != ESUCCESS) {
    printk_error("Failed to trigger isr irq: %x\n", irq);
  } else {
    // trigger the isr
    isr.isr(irq, isr.data);
  }
}

void irq_handle() {
  psw_t psw;
  irq_t irq;

  psw_disable_and_save_interrupt(&psw);
  irq = gic_find_pending_irq();
  if (irq == (uint32_t)IRQ_INVALID) {
    printk_error("INVALID IRQ!\n");
    goto restore_irq_out;
  } else {
    printk_debug("IRQ found: %u 0x%x\n", irq, irq);
  }
  gic_disable_irq(irq);          /* Mask this irq */
  gic_deactivate_interrupt(irq); /* Send EOI for this irq line */
  trigger_isr(irq);
  gic_enable_irq(irq); /* unmask this irq line */

restore_irq_out:
  psw_restore_interrupt(&psw);
}

void common_trap_handler(exception_frame *exc) {
  if ((exc->exc_type & 0xff) == AARCH64_EXC_SYNC_SPX) {
    handle_exception(exc);
  }

  if ((exc->exc_type & 0xff) == AARCH64_EXC_IRQ_SPX) {
    irq_handle();
  }
  return;
}
