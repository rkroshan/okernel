#include "assert.h"
#include "util.h"
void panic(const char *message, const char *file, int line) {
  printk_critical("Kernel Panic: %s at %s:%d\n", message, file, line);
  // Halt the system
  printk_critical("\nSystem halted.\n");
  while (1) {
    __asm__("wfi"); // Wait for interrupt, effectively halting the CPU
  }
}