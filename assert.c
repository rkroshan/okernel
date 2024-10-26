#include "assert.h"
#include "uart.h"
void panic(const char *message, const char *file, int line) {
  printk("Kernel Panic: %s at %s:%d\n", message, file, line);
  // Halt the system
  printk("\nSystem halted.\n");
  while (1) {
    __asm__("wfi"); // Wait for interrupt, effectively halting the CPU
  }
}