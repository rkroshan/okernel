#include "printk.h"
#include "board.h"
#include "spinlock.h"
#include "timer.h"
#include "util.h"
#include <stdarg.h>

/**
 * uart functionalities trying to
 * prevent any source to use it directly
 * in order for synchronisation
 */
extern void uart_putc(const char c);
extern void uart_puthex(uint64_t n);
extern void uart_puts(const char *s);
extern char uart_read();
extern void uart_write(char data);

static DECALRE_SPINLOCK(printk_lock);

extern log_level_e get_current_log_level(void);

const char debug[] = " [DEBUG] ";
const char info[] = " [INFO] ";
const char warn[] = " [WARN] ";
const char error[] = " [!!ERROR!!] ";
const char critical[] = " [!!CRITICAL!!] ";

static int read_string(char *buffer, int position, const char *string) {
  int index = 0;

  for (index = 0; string[index] != '\0'; index++) {
    buffer[position++] = string[index];
  }

  return index;
}

static int hex_to_string(char *buffer, int position, uint64_t digits) {
  char digits_buffer[25];
  char digits_map[16] = "0123456789ABCDEF";
  int size = 0;
  int i = 0;

  do {
    digits_buffer[size++] = digits_map[digits % 16];
    digits /= 16;
  } while (digits != 0);

  for (i = size - 1; i >= 0; i--) {
    buffer[position++] = digits_buffer[i];
  }

  buffer[position++] = 'h';

  return size + 1;
}

static int udecimal_to_string(char *buffer, int position, uint64_t digits) {
  char digits_map[10] = "0123456789";
  char digits_buffer[25];
  int size = 0;
  int i = 0;

  do {
    digits_buffer[size++] = digits_map[digits % 10];
    digits /= 10;
  } while (digits != 0);

  for (i = size - 1; i >= 0; i--) {
    buffer[position++] = digits_buffer[i];
  }

  return size;
}

static int udecimal_to_binary(char *buffer, int position, uint64_t digits) {
  char digits_buffer[100];
  int size = 0;
  int i = 0;
  if (digits == 0) {
    buffer[position++] = 0 + 48;
    size++;
  } else {
    while (digits) {
      uint8_t bit = digits & 1;
      digits_buffer[size++] = bit + 48;
      digits = digits >> 1;
    }
    for (i = size - 1; i >= 0; i--) {
      buffer[position++] = digits_buffer[i];
    }
  }
  return size;
}

static int decimal_to_string(char *buffer, int position, int64_t digits) {
  int size = 0;

  if (digits < 0) {
    digits = -digits;
    buffer[position++] = '-';
    size = 1;
  }

  size += udecimal_to_string(buffer, position, (uint64_t)digits);
  return size;
}

static void write_console(const char *buffer, int size) {
  int i = 0;

  for (i = 0; i < size; i++) {
    uart_write(buffer[i]);
  }
}

int printk(log_level_e log_level, char *format, ...) {
  // print loglevel
  if (log_level < get_current_log_level()) {
    return 0;
  }

  spinlock_acquire(&printk_lock);
  char buffer[LOG_BUFF_SIZE];
  int buffer_size = 0;
  int64_t integer = 0;
  char *string = 0;
  int i = 0;
  va_list args;
  size_t log_level_buffer_size = 0;

  // print timestamp
  uint64_t timens = get_system_timestamp_ns();
  uint64_t times = (timens / 1000000000ULL);
  uint64_t timeus = ((timens % 1000000000ULL) / (1000ULL));
  buffer_size += read_string(buffer, buffer_size, "[");
  buffer_size += udecimal_to_string(buffer, buffer_size, times);
  buffer_size += read_string(buffer, buffer_size, ".");
  buffer_size += udecimal_to_string(buffer, buffer_size, timeus);
  buffer_size += read_string(buffer, buffer_size, "]");

  // cpu info
  buffer_size += read_string(buffer, buffer_size, "CPU");
  buffer_size += udecimal_to_string(buffer, buffer_size, get_current_cpuid());
  buffer_size += read_string(buffer, buffer_size, "");

  switch (log_level) {
  case CRITICAL:
    log_level_buffer_size = strlen(critical);
    memcpy((void *)buffer + buffer_size, (void *)critical,
           log_level_buffer_size);
    break;
  case ERROR:
    log_level_buffer_size = strlen(error);
    memcpy((void *)buffer + buffer_size, (void *)error, log_level_buffer_size);
    break;
  case WARNING:
    log_level_buffer_size = strlen(warn);
    memcpy((void *)buffer + buffer_size, (void *)warn, log_level_buffer_size);
    break;
  case INFO:
    log_level_buffer_size = strlen(info);
    memcpy((void *)buffer + buffer_size, (void *)info, log_level_buffer_size);
    break;
  case DEBUG:
    log_level_buffer_size = strlen(debug);
    memcpy((void *)buffer + buffer_size, (void *)debug, log_level_buffer_size);
    break;
  default:
    // should not happen
    return 0;
  }
  buffer_size += log_level_buffer_size;

  va_start(args, format);

  for (i = 0; format[i] != '\0'; i++) {
    if (buffer_size >= LOG_BUFF_SIZE) {
      // NOT enough space need to increase buf size
      buffer[LOG_BUFF_SIZE - 1] = '\0';
      goto write_now;
    }
    if (format[i] != '%') {
      buffer[buffer_size++] = format[i];
    } else {
      switch (format[++i]) {
      case 'x':
        integer = va_arg(args, int64_t);
        buffer_size += hex_to_string(buffer, buffer_size, (uint64_t)integer) %
                       LOG_BUFF_SIZE;
        break;

      case 'u':
        integer = va_arg(args, int64_t);
        buffer_size +=
            udecimal_to_string(buffer, buffer_size, (uint64_t)integer) %
            LOG_BUFF_SIZE;
        break;

      case 'b':
        integer = va_arg(args, int64_t);
        buffer_size +=
            udecimal_to_binary(buffer, buffer_size, (uint64_t)integer) %
            LOG_BUFF_SIZE;
        break;
      case 'd':
        integer = va_arg(args, int32_t);
        /*https://stackoverflow.com/questions/57748347/why-can-i-import-negative-numbers-as-int-but-not-as-long-int-or-other-larger-da*/
        // integer = va_arg(args, int64_t); /*would be useful when int64 that
        // time suffix "L" at the end to the data*/
        buffer_size +=
            decimal_to_string(buffer, buffer_size, integer) % LOG_BUFF_SIZE;
        break;

      case 's':
        string = va_arg(args, char *);
        buffer_size += read_string(buffer, buffer_size, string) % LOG_BUFF_SIZE;
        break;

      default:
        buffer[buffer_size++] = '%';
        i--;
      }
    }
  }

write_now:
  write_console(buffer, buffer_size);
  va_end(args);
  spinlock_release(&printk_lock);
  return buffer_size;
}