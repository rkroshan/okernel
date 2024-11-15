#include "psci.h"
#include "util.h"
extern void psci_smcc(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3);

/**
 * @brief pci call to turn on cpu
 * not sure but this is now secondary cpus are coming out of reset
 * on qemu-system-aarch64 6.2.0
 *
 * spin table logic not working
 *
 * @param cpuid
 * @param entry_fn
 */
void psci_cpu_on(uint64_t cpuid, uint64_t entry_fn) {
  psci_smcc(PSCI_SYSTEM_CPUON, cpuid, entry_fn, 0);
}

/**
 * @brief shutdown system
 *
 */
void psci_shutdown(void) {
  printk_critical("SYSTEM SHUTDOWN\n");
  psci_smcc(PSCI_SYSTEM_OFF, 0, 0, 0);
}

/**
 * @brief reboot
 *
 */
void psci_reset(void) {
  printk_critical("SYSTEM REBOOT\n");
  psci_smcc(PSCI_SYSTEM_RESET, 0, 0, 0);
}