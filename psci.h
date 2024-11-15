#ifndef __PSCI_H__
#define __PSCI_H__

#include <stdint.h>

#define PSCI_SYSTEM_OFF 0x84000008
#define PSCI_SYSTEM_RESET 0x84000009
#define PSCI_SYSTEM_CPUON 0xc4000003

/**
 * @brief pci call to turn on cpu
 * not sure but this is now secondary cpus are coming out of reset
 * on qemu-system-aarch64 6.2.0
 * also need to call hvc, smc call not working
 * spin table logic not working
 *
 * @param cpuid
 * @param entry_fn
 */
void psci_cpu_on(uint64_t cpuid, uint64_t entry_fn);

/**
 * @brief shutdown system
 *
 */
void psci_shutdown(void);

/**
 * @brief reboot
 *
 */
void psci_reset(void);

#endif