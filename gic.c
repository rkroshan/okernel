#include "gic.h"
#include "aarch64.h"
#include "assert.h"
#include "board.h"
#include "exception.h"
#include "gic_registers.h"
#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
// ------------------------------------------------------------
// Setting location of interfaces
// ------------------------------------------------------------

static struct GICv3_dist_if *gic_dist;
static struct GICv3_rdist_if *gic_rdist;
static uint32_t gic_max_rd =
    0; /*the maximum nuber of gic redistributor supported*/
static uint16_t max_spi_support = 0; /*it is the maximum number of interrupts
                                        supported if espi not supported*/
static uint16_t max_espi_support = 0;

/*TODO support for extended spi/ppi?*/
static isr_struct_t isr_table[MAX_CPUS][GIC_SPI_MAX];

/**
 * @brief Sets the address of the Distributor
 * @param dist   = address of the Distributor
 *
 */
void setGICDAddr(void *dist) {
  gic_dist = (struct GICv3_dist_if *)dist;
  return;
}

/**
 * @brief Sets the address of the ReDistributor for the cpu/vcpu
 * @param rdist  = address of the first RD_base register page
 */
void setGICRAddr(void *rdist) {

  uint32_t index = 0;
  gic_rdist = (struct GICv3_rdist_if *)rdist;
  // Now find the maximum RD ID that I can use
  // This is used for range checking in later functions
  while (
      (gic_rdist[index].lpis.GICR_TYPER[0] & (1 << 4)) ==
      0) // Keep incrementing until GICR_TYPER.Last reports no more RDs in block
  {
    index++;
  }
  gic_max_rd = index + 1;
  printk_debug("Max Number of GIC_RDIST: %u\n", gic_max_rd);
}

/**
 * @brief Get the cpulocal gicr index
 *
 * @return uint32_t
 */
uint32_t get_cpulocal_gicr_index() {
  assert(gic_rdist != NULL);

  uint64_t mpdir = get_mpidr();
  uint8_t aff3 = (mpdir >> 32);
  uint32_t aff = (mpdir & 0xffffff) | (aff3 << 24);

  uint32_t indx = 0;
  while (indx != gic_max_rd) {
    if (gic_rdist[indx].lpis.GICR_TYPER[1] == aff) {
      return indx;
    }
    indx++;
  }

  fatal("no gic redistributor found\n");
  return 0;
}

/**
 * @brief Get the Supported SPI Num
 *
 */
uint16_t getSupportedSPINum() {
  printk_debug("GICD TYPER: %b\n", atomic_load_relaxed(&gic_dist->GICD_TYPER));
  uint8_t itlines = (atomic_load_relaxed(&gic_dist->GICD_TYPER) & (0x1f));
  uint16_t spisnum = (((itlines + 1) << 5) - 1); // 32*(itlines + 1) -1
  printk_debug("Number of SPIs supported: %u\n", spisnum);
  return spisnum >= 1020 ? 1020 : spisnum;
}

/**
 * @brief Get the Supported ESPI Num
 *
 */
uint16_t getSupportedESPINum() {
  if ((atomic_load_relaxed(&gic_dist->GICD_TYPER) >> 8) &
      1) // check if ESPI is supported
  {
    uint8_t eitlines = (atomic_load_relaxed(&gic_dist->GICD_TYPER) >> 27);
    uint16_t espinum = (((eitlines + 1) << 5) + 4095);
    printk_debug("Number of ESPIs supported: %u\n", espinum);
    return espinum;
  } else {
    return 0;
  }
}

/**
 * @brief wait for RWP bit to get clear and prevent reodering of instructions
 *
 * @return uint32_t
 */
static uint32_t gicd_wait_for_write(void) {
  atomic_thread_fence(memory_order_seq_cst);
  uint32_t ctlr = atomic_load_relaxed(&gic_dist->GICD_CTLR);

  while ((ctlr & (1 << 31)) != 0U) {
    ctlr = atomic_load_relaxed(&gic_dist->GICD_CTLR);
  }
  return ctlr;
}

/**
 * @brief wait for RWP bit to get clear and prevent reodering of instructions
 *
 * @param indx
 * @return uint32_t
 */
static uint32_t gicr_wait_for_write(uint32_t indx) {
  atomic_thread_fence(memory_order_seq_cst);
  uint32_t ctlr = atomic_load_relaxed(&gic_rdist[indx].lpis.GICR_CTLR);

  while ((ctlr & (1 << 3)) != 0U) {
    ctlr = atomic_load_relaxed(&gic_rdist[indx].lpis.GICR_CTLR);
  }
  return ctlr;
}

/**
 * @brief wait for upstream write to get communicated to distributor
 * useful when generating SGIs and needs to make sure it is communicated to
 * distributor
 *
 * @param indx
 * @return uint32_t
 */
static uint32_t gicr_wait_for_upstream_write(uint32_t indx) {
  atomic_thread_fence(memory_order_seq_cst);
  uint32_t ctlr = atomic_load_relaxed(&gic_rdist[indx].lpis.GICR_CTLR);

  while ((ctlr & (1 << 31)) != 0U) {
    ctlr = atomic_load_relaxed(&gic_rdist[indx].lpis.GICR_CTLR);
  }
  return ctlr;
}

/**
 * @brief enable gicr redistributor of the cpu/vcpu
 *
 * @param gicr_indx
 */
void gicr_enable(uint32_t gicr_indx) {
  uint32_t waker = atomic_load_relaxed(&gic_rdist[gicr_indx].lpis.GICR_WAKER);
  /*check id redistributor is already enabled
  we do this by checking the ChildAsSleep bit in WAKER register if cleared
  already then return*/
  if ((waker & (1 << 2)) == 0) {
    return;
  }

  /*clear the ProcessorSleep bit and wait for ChildAsAleep bit be cleared*/
  waker = waker & ~((uint32_t)1 << 1);
  atomic_store_relaxed(&gic_rdist[gicr_indx].lpis.GICR_WAKER, waker);
  (void)gicr_wait_for_write(gicr_indx);
  printk_debug("gicr_indx %x waker: %b\n", gicr_indx,
               atomic_load_relaxed(&gic_rdist[gicr_indx].lpis.GICR_WAKER));
  while ((atomic_load_relaxed(&gic_rdist[gicr_indx].lpis.GICR_WAKER) &
          (1 << 2)) != 0) {
  }
}

/**
 * @brief Initialize GIC Redistributor interface for the cpu/vcpu
 *
 */
void init_gicr() {
  printk_debug("init_gicr()\n");
  /*set gicr address*/
  setGICRAddr((void *)GIC_REDIST);
  /*get the cpu local index for gicr*/
  uint32_t gicr_indx = get_cpulocal_gicr_index();
  printk_debug("GICR Reset Val: %b\n",
               atomic_load_relaxed(&gic_rdist[gicr_indx].lpis.GICR_CTLR));

  /*disable all SGIs PPIs*/
  atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_ICENABLER[0],
                       0xffffffffU);
  (void)gicr_wait_for_write(gicr_indx);

  /*set default priority for SGI/PPIs*/
  for (uint32_t i = 0; i < GIC_GICR_PPI_MAX; i++) {
    atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_IPRIORITYR[i],
                         GIC_SGI_PPI_DEFAULT_PRIORITY);
  }
  (void)gicr_wait_for_write(gicr_indx);

  /*set PPI trigger as level*/
  atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_ICFGR[1],
                       (uint32_t)GIC_ICFGR_LEVEL);
  (void)gicr_wait_for_write(gicr_indx);

  /*set SGI/PPIs interrupts as NSG1, i.e IGROUPR[i]=1, IGRPMODR[i]=0
  we are assuming if DS=0 as well so will set IGRPMODR reg as well
  if DS=1, then it will be RES0*/
  atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_IGROUPR[0], 0xffffffffU);
  atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_IGRPMODR[0],
                       (uint32_t)0);
  (void)gicr_wait_for_write(gicr_indx);

  /*enable the redistributor*/
  gicr_enable(gicr_indx);
}

/**
 * @brief Initialize GIC Controller interface for the cpu/vcpu
 *
 */
void init_gicc(void) {
  printk_debug("init_gicc()\n");
  /*affinity routing is enabled so we can access cpu interface registers via
   * system register access*/
  uint64_t icc_sre_el1;
  ASM_MRS_ICC_SRE_EL1(icc_sre_el1);
  printk_debug("GIC ICC Reset Val: %b\n", icc_sre_el1);

  /*if system register access is disabled we will assert to enable it
  Also we disable IRQ and FIQ bypass so that every interrupt go through GIC
  controller*/
  if (!(icc_sre_el1 & ICC_SRE_ELx_SRE_BIT)) {
    icc_sre_el1 = (icc_sre_el1 | ICC_SRE_ELx_SRE_BIT | ICC_SRE_ELx_DFB_BIT |
                   ICC_SRE_ELx_DIB_BIT);
    ASM_MSR_ICC_SRE_EL1(icc_sre_el1);
    ASM_MRS_ICC_SRE_EL1(icc_sre_el1); /*read it again to make sure it is set*/
    assert(icc_sre_el1 & ICC_SRE_ELx_SRE_BIT);
  }

  /*set lowest priority to allow all interrupts*/
  uint64_t priority = 0xff;
  ASM_MSR_ICC_PMR_EL1(priority);

  /*we will not set BPR to 0x7 since we require preemption of important
  interrupts we want timer interrupt and other important interrupt to preempt
  the kernel so we will set timer interrupt at 0x20 with BPR set to 0x3, so that
  4 upper bits decide the preemtion*/
  uint64_t bpr = 0x3;
  ASM_MSR_ICC_BPR1_EL1(bpr);

  /*clear EOImode bit to drop interrupt priority and deactivate interrupt at
   * once*/
  uint64_t icc_ctlr;
  ASM_MRS_ICC_CTLR_EL1(icc_ctlr);
  icc_ctlr &= ~(1 << 1);
  ASM_MSR_ICC_CTLR_EL1(icc_ctlr);

  /*enable group 1 interrupts*/
  uint64_t enable_grp1_int = 0x1;
  ASM_MSR_ICC_IGRPEN1_EL1(enable_grp1_int);
}

/**
 * @brief Initialize gicv3 distributor
 * @param None
 * @return
 */
void init_gicd(void) {
  printk_debug("init_gicd()\n");
  /*set GIC Addr*/
  setGICDAddr((void *)GIC_BASE);
  printk_debug("GICD Reset Val: %b\n",
               atomic_load_relaxed(&gic_dist->GICD_IROUTER[32]));
  /*we are setting interrupts in a way that even if DS is set to 0/1
  all interrupts should be treated as NS G1 interrupts*/

  /*the number of supported SPIs*/
  max_spi_support = getSupportedSPINum();
  max_espi_support = getSupportedESPINum();

  /* Disable distributor */
  uint32_t ctlr = 0U;
  atomic_store_relaxed(&gic_dist->GICD_CTLR, ctlr);
  ctlr = gicd_wait_for_write();

  // Enable affinity routing
  /*so if DS=0, then setting ARE_S should be suffient although
  no harm setting ARE_NS since it will be WI*/
  ctlr |= (1 << 5) | (1 << 4);
  atomic_store_relaxed(&gic_dist->GICD_CTLR, ctlr);
  ctlr = gicd_wait_for_write();

  for (uint32_t i = 1U; i < (max_spi_support / GIC_GICD_INT_PER_REG); i++) {
    // Disable all SPIs
    atomic_store_relaxed(&gic_dist->GICD_ICENABLER[i], 0xffffffffU);
  }
  ctlr = gicd_wait_for_write();

  for (uint32_t i = 1U; i < (max_spi_support / GIC_GICD_INT_PER_REG); i++) {
    // clear all pending interrupts
    atomic_store_relaxed(&gic_dist->GICD_ICPENDR[i], 0xffffffffU);
  }
  ctlr = gicd_wait_for_write();

  for (uint32_t i = 1U; i < (max_spi_support / GIC_GICD_INT_PER_REG); i++) {
    /*set all SPI interrupts as NSG1, i.e IGROUPR[i]=1, IGRPMODR[i]=0
    we are assuming if DS=0 as well so will set IGRPMODR reg as well
    if DS=1, then it will be RES0*/
    atomic_store_relaxed(&gic_dist->GICD_IGROUPR[i], 0xffffffffU);
    atomic_store_relaxed(&gic_dist->GICD_IGRPMODR[i], (uint32_t)0);
  }
  ctlr = gicd_wait_for_write();

  for (uint32_t i = GIC_SPI_BASE; i < max_spi_support; i++) {
    /*set up all SPI with default priority of 0xA0*/
    /*
    A common priority scheme in a GIC implementation might look like this:
    0x00 to 0x3F: Critical and high-priority interrupts (e.g., SGIs, important
    system timers). 0x40 to 0x9F: Medium-priority interrupts. 0xA0 to 0xDF:
    Standard device interrupts (default SPI priority). 0xE0 to 0xFF:
    Low-priority or background interrupts.
    */
    atomic_store_relaxed(&gic_dist->GICD_IPRIORITYR[i],
                         GIC_SPI_DEFAULT_PRIORITY);
  }
  ctlr = gicd_wait_for_write();

  for (uint32_t i = 1; i < max_spi_support / GIC_GICD_ICFGR_PER_REG; i++) {
    /*set all spis as level triggered*/
    atomic_store_relaxed(&gic_dist->GICD_ICFGR[i], (uint32_t)GIC_ICFGR_LEVEL);
  }
  ctlr = gicd_wait_for_write();

  uint64_t mpidr = get_mpidr();
  /*value is in the format aff3:0x00:aff2:aff1:aff0,
  since interrupt routing mode in based on affinity so no change*/
  for (uint32_t i = GIC_SPI_BASE; i < max_spi_support; i++) {
    /*set default routing of all spis as this cpu mpidr value*/
    atomic_store_relaxed(&gic_dist->GICD_IROUTER[i], mpidr);
  }
  ctlr = gicd_wait_for_write();

  /*enable NSG1 irqs in distributor,
  EnableGrp1NS will enable NSG1, also enabling GS1 and GS0 to think of DS=0*/
  ctlr |= (1 << 0) | (1 << 1) | (1 << 2);
  atomic_store_relaxed(&gic_dist->GICD_CTLR, ctlr);
}

/** Disable IRQ
        @param[in] irq IRQ number
 */
void gic_disable_irq(irq_t irq) {
  if ((irq >= GIC_SGI_BASE) && (irq < GIC_GICR_PPI_MAX)) {
    uint32_t gicr_indx = get_cpulocal_gicr_index();
    /*ICEnabler register when writes 1 in the irq index disable that irq, read
     * has no effect and will return 0 */
    atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_ICENABLER[0],
                         UBIT(irq));
    gicr_wait_for_write(gicr_indx);
  } else if ((irq >= GIC_SPI_BASE) && (irq < GIC_SPI_MAX)) {
    atomic_store_relaxed(&gic_dist->GICD_ICENABLER[GICD_ENABLE_GET_N(irq)],
                         UBIT(irq));
    gicd_wait_for_write();
  } else {
    /*TODO: it can be extended spis or ppis*/
  }
}

/** Enable IRQ
        @param[in] irq IRQ number
 */
void gic_enable_irq(irq_t irq) {
  if ((irq >= GIC_SGI_BASE) && (irq < GIC_GICR_PPI_MAX)) {
    uint32_t gicr_indx = get_cpulocal_gicr_index();
    atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_ISENABLER[0],
                         UBIT(irq));
  } else if ((irq >= GIC_SPI_BASE) && (irq < GIC_SPI_MAX)) {
    atomic_store_relaxed(&gic_dist->GICD_ISENABLER[GICD_ENABLE_GET_N(irq)],
                         UBIT(irq));
  } else {
    /*TODO: it can be extended spis or ppis*/
  }
}

/** Clear a pending interrupt
        @param[in] irq IRQ number
 */
void gic_clear_pending(irq_t irq) {
  if ((irq >= GIC_SGI_BASE) && (irq < GIC_GICR_PPI_MAX)) {
    uint32_t gicr_indx = get_cpulocal_gicr_index();
    atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_ICPENDR[0], UBIT(irq));
  } else if ((irq >= GIC_SPI_BASE) && (irq < GIC_SPI_MAX)) {
    atomic_store_relaxed(&gic_dist->GICD_ICPENDR[GICD_ENABLE_GET_N(irq)],
                         UBIT(irq));
  } else {
    /*TODO: it can be extended spis or ppis*/
  }
}

/**
 * @brief get the irq ID and acknowledge it
 *
 * @return (uint64_t) irq ID
 */
static uint64_t gic_acknowledge_irq() {
  uint64_t icc_iar1;
  ASM_MRS_ICC_IAR1_EL1(icc_iar1);
  // Ensure IRQ is not handled before it is acknowledged in the GICD
  BARRIER();
  return (icc_iar1 < (uint64_t)GIC_SPI_MAX) ? (uint64_t)icc_iar1 : IRQ_INVALID;
}

/**
 * @brief Set an interrupt priority
 *
 * @param irq   IRQ number
 * @param prio  Interrupt priority in Arm specific expression
 */
void gic_set_priority(irq_t irq, uint8_t prio) {
  if ((irq >= GIC_SGI_BASE) && (irq < GIC_GICR_PPI_MAX)) {
    uint32_t gicr_indx = get_cpulocal_gicr_index();
    atomic_store_relaxed(&gic_rdist[gicr_indx].sgis.GICR_IPRIORITYR[irq], prio);
    gicr_wait_for_write(gicr_indx);
  } else if ((irq >= GIC_SPI_BASE) && (irq < GIC_SPI_MAX)) {
    atomic_store_relaxed(&gic_dist->GICD_IPRIORITYR[irq], prio);
    gicd_wait_for_write();
  } else {
    /*TODO: it can be extended spis or ppis*/
  }
}

/**
 * @brief Configure IRQ trigger as LEVEL or EDGE Sensitive
 *
 * @param irq     IRQ number
 * @param config  Configuration value for GICD_ICFGR
 */
void gic_set_irq_cfg(irq_t irq, uint8_t config) {
  uint32_t curr_icfg;
  config = config & 0x3U;
  assert((config == GIC_ICFGR_LEVEL) || (config == GIC_ICFGR_EDGE));

  if ((irq >= GIC_SGI_BASE) && (irq < GIC_SGI_MAX)) {
    /*No need to set SGIs are always edge triggered*/
  } else if ((irq >= GIC_PPI_BASE) && (irq < GIC_GICR_PPI_MAX)) {
    uint32_t gicr_indx = get_cpulocal_gicr_index();
    curr_icfg = atomic_load_relaxed(
        &gic_rdist[gicr_indx].sgis.GICR_ICFGR[GIC_ICFGR_GET_N(irq)]);
    if (config == GIC_ICFGR_LEVEL) {
      curr_icfg =
          curr_icfg & ~(0x3 << ((irq - (GIC_ICFGR_GET_N(irq)) * 16U) * 2U));
    } else {
      /*it should be edge*/
      curr_icfg =
          curr_icfg | (0x2 << ((irq - (GIC_ICFGR_GET_N(irq)) * 16U) * 2U));
    }
    atomic_store_relaxed(
        &gic_rdist[gicr_indx].sgis.GICR_ICFGR[GIC_ICFGR_GET_N(irq)], curr_icfg);
    gicr_wait_for_write(gicr_indx);
  } else if ((irq >= GIC_SPI_BASE) && (irq < GIC_SPI_MAX)) {
    curr_icfg =
        atomic_load_relaxed(&gic_dist->GICD_ICFGR[GIC_ICFGR_GET_N(irq)]);
    if (config == GIC_ICFGR_LEVEL) {
      curr_icfg =
          curr_icfg & ~(0x3 << ((irq - (GIC_ICFGR_GET_N(irq)) * 16U) * 2U));
    } else {
      /*it should be edge*/
      curr_icfg =
          curr_icfg | (0x2 << ((irq - (GIC_ICFGR_GET_N(irq)) * 16U) * 2U));
    }
    atomic_store_relaxed(&gic_dist->GICD_ICFGR[GIC_ICFGR_GET_N(irq)],
                         curr_icfg);
    gicd_wait_for_write();
  } else {
    /*TODO: it can be extended spis or ppis*/
  }
}

/** Send End of Interrupt to ICC interface that it has completed the processing
   of specified group1 interrupt
        @param[in] ctrlr   IRQ controller information
        @param[in] irq     IRQ number
 */
static void gic_eoir1_el1(irq_t irq) {
  uint64_t eoir1_el1 = (uint64_t)irq;
  data_barrier();
  ASM_MSR_ICC_EOIR1_EL1(eoir1_el1);
}

static void _gic_deactivate_interrupt(irq_t irq) {
  uint64_t dir_el1 = (uint64_t)irq;
  data_barrier();
  ASM_MSR_ICC_DIR_EL1(dir_el1);
}

/**
 * @brief deactivate interrupt from cpu interface
 *
 * @param irq
 */
void gic_deactivate_interrupt(irq_t irq) {
  assert(irq >= GIC_SGI_BASE);
  assert(irq < GIC_SPI_MAX);
  // TODO: what if extended SPI or PPI?

  /*check the EOI mode*/
  uint64_t icc_ctlr_el1;
  ASM_MRS_ICC_CTLR_EL1(icc_ctlr_el1);
  uint8_t eoi_mode = (icc_ctlr_el1 & (1 << 1)) ? 1U : 0U;

  if (eoi_mode) {
    /*need to drop priority*/
    gic_eoir1_el1(irq);
    /*deactivate seperately*/
    _gic_deactivate_interrupt(irq);
  } else {
    /*drop and activate happens just by writing to eoi reg*/
    gic_eoir1_el1(irq);
  }
}

/*
 * @brief Initialize GIC V3 IRQ controller
 *
 * @param None
 * @return
 */
void gic_v3_initialize(void) {
  printk_debug("gic_v3_initialize()\n");
  init_gicd();
  init_gicr();
  init_gicc();
}

/*
 * @brief Initialize GIC V3 IRQ controller for secondary cores
 *
 * @param None
 * @return
 */
void gic_v3_initialize_secondary(void) {
  printk_debug("gic_v3_initialize_secondary()\n");
  init_gicr();
  init_gicc();
}

/**
 * @brief Find pending IRQ
 *
 * @return irq_t
 */
irq_t gic_find_pending_irq() { return (uint32_t)gic_acknowledge_irq(); }

/**
 * @brief core0 intialise interrupt controller
 *
 */
void primary_init_interrupt_controller(void) {
#ifdef GIC_V3
  gic_v3_initialize();
#else
#error "gic_v3 interrupt controller is implemented as of now"
#endif
}

/**
 * @brief secondary core intialise interrupt controller
 *
 */
void secondary_init_interrupt_controller(void) {
#ifdef GIC_V3
  gic_v3_initialize_secondary();
#else
#error "gic_v3 interrupt controller is implemented as of now"
#endif
}

/**
 * @brief register for interrupt isr handler
 *
 * @param irq
 * @param isr
 * @param data
 * @return uint8_t
 */
uint8_t register_interrupt_isr(irq_t irq, isr_t isr, void *data) {
  uint8_t ret;
  uint64_t cpuid = get_current_cpuid();

  if (irq >= GIC_SPI_MAX) {
    ret = EINVALID;
    printk_error("Failed to register isr due to invalid irq : %x\n", irq);
    goto out;
  }

  if (isr_table[cpuid][irq].isr != NULL) {
    ret = EINVALID;
    printk_error("Failed to register isr, isr already register for irq: %x\n",
                 irq);
    goto out;
  }

  isr_table[cpuid][irq].isr = isr;
  isr_table[cpuid][irq].data = data;
  ret = ESUCCESS;

out:
  return ret;
}

/**
 * @brief Get the registered isr
 *
 * @param irq
 * @param isr
 * @return uint8_t ret value
 */
uint8_t get_registered_isr(irq_t irq, isr_struct_t *isr) {
  uint8_t ret;
  uint64_t cpuid = get_current_cpuid();
  if (irq >= GIC_SPI_MAX) {
    ret = EINVALID;
    isr->isr = NULL;
    printk_error("get_registered_isr: Failed: irq invalid: %x\n", irq);
    goto out;
  }

  if (isr_table[cpuid][irq].isr == NULL) {
    ret = EINVALID;
    printk_error("get_registered_isr: Failed: no isr registered: %x\n", irq);
    goto out;
  }

  *isr = isr_table[cpuid][irq];
  ret = ESUCCESS;

out:
  return ret;
}
