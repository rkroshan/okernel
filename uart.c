#include "board.h"
#include <stdint.h>
volatile unsigned int *const UART0DR = (unsigned int *)VIRT_UART_ADDR;
volatile unsigned int *const UART0FR = (unsigned int *)VIRT_UART_ADDR + 0x18;

void uart_putc(const char c) {
  // Wait for UART to become ready to transmit.
  while ((*UART0FR) & (1 << 5)) {
  }
  *UART0DR = c; /* Transmit char */
}

void uart_puthex(uint64_t n) {
  const char *hexdigits = "0123456789ABCDEF";

  uart_putc('0');
  uart_putc('x');
  for (int i = 60; i >= 0; i -= 4) {
    uart_putc(hexdigits[(n >> i) & 0xf]);
    if (i == 32)
      uart_putc(' ');
  }
}

void uart_puts(const char *s) {
  for (int i = 0; s[i] != '\0'; i++)
    uart_putc((unsigned char)s[i]);
}

char uart_read() { return (char)*UART0DR; }

void uart_write(char data) { uart_putc(data); }
