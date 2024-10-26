/* -*- mode: c; coding:utf-8 -*- */
/**********************************************************************/
/*  OS kernel sample                                                  */
/*  Copyright 2014 Takeharu KATO                                      */
/*                                                                    */
/*  PrimeCell Generic Interrupt Controller (PL390)                    */
/*                                                                    */
/**********************************************************************/

/**********************************************************************/
/*  armv8-bare-metal                                                  */
/*  Copyright 2018 Nienfeng Yao                                       */
/*                                                                    */
/*  Reference: ARM® Generic Interrupt Controller Architecture         */
/*      Specification GIC architecture version 3.0 and version 4.0    */
/*                                                                    */
/**********************************************************************/
#if !defined(_GIC_V3_H)
#define _GIC_V3_H

#include "exception.h"
#include "util.h"

typedef uint32_t irq_no; /* IRQ no */

#define GIC_GICD_BASE (GIC_BASE)           /* GICD MMIO base address */
#define GIC_GICC_BASE (GIC_BASE + 0x10000) /* GICC MMIO base address */

#define IRQ_INVALID ~(uint64_t)0U

#define GIC_SGI_BASE (0)
#define GIC_PPI_BASE (16)
#define GIC_SPI_BASE (32)
#define GIC_SPI_DEFAULT_PRIORITY (0xA0U)
#define GIC_SGI_PPI_DEFAULT_PRIORITY GIC_SPI_DEFAULT_PRIORITY
#define GIC_GICD_INT_PER_REG (32)      /* 32 interrupts per reg */
#define GIC_GICD_IPRIORITY_PER_REG (4) /* 4 priority per reg */
#define GIC_GICR_IPRIORITY_PER_REG GIC_GICD_IPRIORITY_PER_REG
#define GIC_GICD_IPRIORITY_SIZE_PER_REG (8) /* priority element size */
#define GIC_GICD_ITARGETSR_CORE0_TARGET_BMAP                                   \
  (0x01010101) /* CPU interface 0                                              \
                */
#define GIC_GICD_ITARGETSR_PER_REG (4)
#define GIC_GICD_ITARGETSR_SIZE_PER_REG (8)
#define GIC_GICD_ICFGR_PER_REG (16)
#define GIC_GICD_ICFGR_SIZE_PER_REG (2)
#define GIC_GICD_ICENABLER_PER_REG (32)
#define GIC_GICD_ISENABLER_PER_REG (32)
#define GIC_GICD_ICPENDR_PER_REG (32)
#define GIC_GICD_ISPENDR_PER_REG (32)
#define GIC_SGI_MAX (16)
#define GIC_GICR_PPI_MAX (32)
#define GIC_SPI_MAX (1020)

/* 8.12 The GIC CPU interface register map */
#define GIC_GICC_CTLR                                                          \
  (GIC_GICC_BASE + 0x000) /* CPU Interface Control Register */
#define GIC_GICC_PMR                                                           \
  (GIC_GICC_BASE + 0x004) /* Interrupt Priority Mask Register */
#define GIC_GICC_BPR (GIC_GICC_BASE + 0x008) /* Binary Point Register */
#define GIC_GICC_IAR                                                           \
  (GIC_GICC_BASE + 0x00C) /* Interrupt Acknowledge Register */
#define GIC_GICC_EOIR (GIC_GICC_BASE + 0x010) /* End of Interrupt Register */
#define GIC_GICC_RPR (GIC_GICC_BASE + 0x014)  /* Running Priority Register */
#define GIC_GICC_HPIR                                                          \
  (GIC_GICC_BASE + 0x018) /* Highest Pending Interrupt Register */
#define GIC_GICC_ABPR                                                          \
  (GIC_GICC_BASE + 0x01C) /* Aliased Binary Point Register */
#define GIC_GICC_IIDR                                                          \
  (GIC_GICC_BASE + 0x0FC) /* CPU Interface Identification Register */

/* 8.13.7 GICC_CTLR, CPU Interface Control Register */
#define GICC_CTLR_ENABLE (0x1)  /* Enable GICC */
#define GICC_CTLR_DISABLE (0x0) /* Disable GICC */

/* 8.13.14 GICC_PMR, CPU Interface Priority Mask Register */
#define GICC_PMR_PRIO_MIN (0xff) /* The lowest level mask */
#define GICC_PMR_PRIO_HIGH (0x0) /* The highest level mask */

/* 8.13.6 GICC_BPR, CPU Interface Binary Point Register */
/* In systems that support only one Security state, when GICC_CTLR.CBPR == 0,
this register determines only Group 0 interrupt preemption. */
#define GICC_BPR_NO_GROUP (0x0) /* handle all interrupts */

/* 8.13.11 GICC_IAR, CPU Interface Interrupt Acknowledge Register */
#define GICC_IAR_INTR_IDMASK (0x3ff)   /* 0-9 bits means Interrupt ID */
#define GICC_IAR_SPURIOUS_INTR (0x3ff) /* 1023 means spurious interrupt */

/* 8.8 The GIC Distributor register map */
#define GIC_GICD_CTLR                                                          \
  (GIC_GICD_BASE + 0x000) /* Distributor Control Register                      \
                           */
#define GIC_GICD_TYPER                                                         \
  (GIC_GICD_BASE + 0x004) /* Interrupt Controller Type Register */
#define GIC_GICD_IIDR                                                          \
  (GIC_GICD_BASE + 0x008) /* Distributor Implementer Identification Register   \
                           */
#define GIC_GICD_IGROUPR(n)                                                    \
  (GIC_GICD_BASE + 0x080 + ((n)*4)) /* Interrupt Group Registers */
#define GIC_GICD_ISENABLER(n)                                                  \
  (GIC_GICD_BASE + 0x100 + ((n)*4)) /* Interrupt Set-Enable Registers */
#define GIC_GICD_ICENABLER(n)                                                  \
  (GIC_GICD_BASE + 0x180 + ((n)*4)) /* Interrupt Clear-Enable Registers */
#define GIC_GICD_ISPENDR(n)                                                    \
  (GIC_GICD_BASE + 0x200 + ((n)*4)) /* Interrupt Set-Pending Registers */
#define GIC_GICD_ICPENDR(n)                                                    \
  (GIC_GICD_BASE + 0x280 + ((n)*4)) /* Interrupt Clear-Pending Registers */
#define GIC_GICD_ISACTIVER(n)                                                  \
  (GIC_GICD_BASE + 0x300 + ((n)*4)) /* Interrupt Set-Active Registers */
#define GIC_GICD_ICACTIVER(n)                                                  \
  (GIC_GICD_BASE + 0x380 + ((n)*4)) /* Interrupt Clear-Active Registers */
#define GIC_GICD_IPRIORITYR(n)                                                 \
  (GIC_GICD_BASE + 0x400 + ((n)*4)) /* Interrupt Priority Registers */
#define GIC_GICD_ITARGETSR(n)                                                  \
  (GIC_GICD_BASE + 0x800 + ((n)*4)) /* Interrupt Processor Targets Registers   \
                                     */
#define GIC_GICD_ICFGR(n)                                                      \
  (GIC_GICD_BASE + 0xc00 + ((n)*4)) /* Interrupt Configuration Registers */
#define GIC_GICD_NSCAR(n)                                                      \
  (GIC_GICD_BASE + 0xe00 + ((n)*4)) /* Non-secure Access Control Registers */
#define GIC_GICD_SGIR                                                          \
  (GIC_GICD_BASE + 0xf00) /* Software Generated Interrupt Register */
#define GIC_GICD_CPENDSGIR(n)                                                  \
  (GIC_GICD_BASE + 0xf10 + ((n)*4)) /* SGI Clear-Pending Registers */
#define GIC_GICD_SPENDSGIR(n)                                                  \
  (GIC_GICD_BASE + 0xf20 + ((n)*4)) /* SGI Set-Pending Registers */

/* 8.9.4 GICD_CTLR, Distributor Control Register */
#define GIC_GICD_CTLR_ENABLE (0x1)  /* Enable GICD */
#define GIC_GICD_CTLR_DISABLE (0x0) /* Disable GICD */

/* 8.9.7 GICD_ICFGR<n>, Interrupt Configuration Registers */
#define GIC_ICFGR_LEVEL (0x0) /* level-sensitive */
#define GIC_ICFGR_EDGE (0x2)  /* edge-triggered */

/* System Register access macros for GICC */
#define ASM_MSR_ICC_SRE_EL1(a)                                                 \
  __asm__ volatile("msr ICC_SRE_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_BPR0_EL1(a)                                                \
  __asm__ volatile("msr ICC_BPR0_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_BPR1_EL1(a)                                                \
  __asm__ volatile("msr ICC_BPR1_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_CTLR_EL1(a)                                                \
  __asm__ volatile("msr ICC_CTLR_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_DIR_EL1(a)                                                 \
  __asm__ volatile("msr ICC_DIR_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_EOIR0_EL1(a)                                               \
  __asm__ volatile("msr ICC_EOIR0_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_EOIR1_EL1(a)                                               \
  __asm__ volatile("msr ICC_EOIR1_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_HPPIR0_EL1(a)                                              \
  __asm__ volatile("msr ICC_HPPIR0_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_HPPIR1_EL1(a)                                              \
  __asm__ volatile("msr ICC_HPPIR1_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_IAR0_EL1(a)                                                \
  __asm__ volatile("msr ICC_IAR0_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_IAR1_EL1(a)                                                \
  __asm__ volatile("msr ICC_IAR1_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_IGRPEN0_EL1(a)                                             \
  __asm__ volatile("msr ICC_IGRPEN0_EL0, %0" ::"r"((a)))
#define ASM_MSR_ICC_IGRPEN1_EL1(a)                                             \
  __asm__ volatile("msr ICC_IGRPEN1_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_NMIAR1_EL1(a)                                              \
  __asm__ volatile("msr ICC_NMIAR1_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_PMR_EL1(a)                                                 \
  __asm__ volatile("msr ICC_PMR_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_RPR_EL1(a)                                                 \
  __asm__ volatile("msr ICC_RPR_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_SGI0R_EL1(a)                                               \
  __asm__ volatile("msr ICC_SGI0R_EL1, %0" ::"r"((a)))
#define ASM_MSR_ICC_SGI1R_EL1(a)                                               \
  __asm__ volatile("msr ICC_SGI1R_EL1, %0" ::"r"((a)))

#define ASM_MRS_ICC_SRE_EL1(a)                                                 \
  __asm__ volatile("mrs %0, ICC_SRE_EL1" : "=r"((a)))
#define ASM_MRS_ICC_BPR0_EL1(a)                                                \
  __asm__ volatile("mrs %0, ICC_BPR0_EL1" : "=r"((a)))
#define ASM_MRS_ICC_BPR1_EL1(a)                                                \
  __asm__ volatile("mrs %0, ICC_BPR1_EL1" : "=r"((a)))
#define ASM_MRS_ICC_CTLR_EL1(a)                                                \
  __asm__ volatile("mrs %0, ICC_CTLR_EL1" : "=r"((a)))
#define ASM_MRS_ICC_DIR_EL1(a)                                                 \
  __asm__ volatile("mrs %0, ICC_DIR_EL1" : "=r"((a)))
#define ASM_MRS_ICC_EOIR0_EL1(a)                                               \
  __asm__ volatile("mrs %0, ICC_EOIR0_EL1" : "=r"((a)))
#define ASM_MRS_ICC_EOIR1_EL1(a)                                               \
  __asm__ volatile("mrs %0, ICC_EOIR1_EL1" : "=r"((a)))
#define ASM_MRS_ICC_HPPIR0_EL1(a)                                              \
  __asm__ volatile("mrs %0, ICC_HPPIR0_EL1" : "=r"((a)))
#define ASM_MRS_ICC_HPPIR1_EL1(a)                                              \
  __asm__ volatile("mrs %0, ICC_HPPIR1_EL1" : "=r"((a)))
#define ASM_MRS_ICC_IAR0_EL1(a)                                                \
  __asm__ volatile("mrs %0, ICC_IAR0_EL1" : "=r"((a)))
#define ASM_MRS_ICC_IAR1_EL1(a)                                                \
  __asm__ volatile("mrs %0, ICC_IAR1_EL1" : "=r"((a)))
#define ASM_MRS_ICC_IGRPEN0_EL1(a)                                             \
  __asm__ volatile("mrs %0, ICC_IGRPEN0_EL0" : "=r"((a)))
#define ASM_MRS_ICC_IGRPEN1_EL1(a)                                             \
  __asm__ volatile("mrs %0, ICC_IGRPEN1_EL1" : "=r"((a)))
#define ASM_MRS_ICC_NMIAR1_EL1(a)                                              \
  __asm__ volatile("mrs %0, ICC_NMIAR1_EL1" : "=r"((a)))
#define ASM_MRS_ICC_PMR_EL1(a)                                                 \
  __asm__ volatile("mrs %0, ICC_PMR_EL1" : "=r"((a)))
#define ASM_MRS_ICC_RPR_EL1(a)                                                 \
  __asm__ volatile("mrs %0, ICC_RPR_EL1" : "=r"((a)))
#define ASM_MRS_ICC_SGI0R_EL1(a)                                               \
  __asm__ volatile("mrs %0, ICC_SGI0R_EL1" : "=r"((a)))
#define ASM_MRS_ICC_SGI1R_EL1(a)                                               \
  __asm__ volatile("mrs %0, ICC_SGI1R_EL1" : "=r"((a)))

/* register constants */
#define ICC_SRE_ELx_SRE_BIT BIT(0)
#define ICC_SRE_ELx_DFB_BIT BIT(1)
#define ICC_SRE_ELx_DIB_BIT BIT(2)
#define ICC_SRE_EL3_EN_BIT BIT(3)

/*GIC utility functions*/
#define GICD_ENABLE_GET_N(x) ((x) >> 5)
#define GIC_ICFGR_GET_N(x) ((x) >> 4)

void gic_v3_initialize(void);
void gic_deactivate_interrupt(irq_no irq);
irq_no gic_v3_find_pending_irq(void);
void gic_disable_irq(irq_no irq);
void gic_enable_irq(irq_no irq);
void gic_clear_pending(irq_no irq);
#endif /* _GIC_V3_H */
