#if !defined(_UART_H)
#define  _UART_H 
#include <stdint.h>

void uart_putc(const char c);
void uart_puthex(uint64_t n);
void uart_puts(const char *s);

char uart_read();
void uart_write(char data);

int printk(const char *format, ...);
#endif  /*  _UART_H   */
