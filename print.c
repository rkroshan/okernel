#include "uart.h"
#include <stdarg.h>

static int read_string(char *buffer, int position, const char *string)
{
    int index = 0;

    for (index = 0; string[index] != '\0'; index++) {
        buffer[position++] = string[index];
    }

    return index;
}

static int hex_to_string(char *buffer, int position, uint64_t digits)
{
    char digits_buffer[25];
    char digits_map[16] = "0123456789ABCDEF";
    int size = 0;
    int i = 0;

    do {
        digits_buffer[size++] = digits_map[digits % 16];
        digits /= 16;
    } while (digits != 0);

    for (i = size-1; i >= 0; i--) {
        buffer[position++] = digits_buffer[i];
    }

    buffer[position++] = 'H';

    return size + 1;
}

static int udecimal_to_string(char *buffer, int position, uint64_t digits)
{
    char digits_map[10] = "0123456789";
    char digits_buffer[25];
    int size = 0;
    int i = 0;

    do {
        digits_buffer[size++] = digits_map[digits % 10];
        digits /= 10;
    } while (digits != 0);

    for (i = size-1; i >= 0; i--) {
        buffer[position++] = digits_buffer[i];
    }

    return size;
}

static int udecimal_to_binary(char *buffer, int position, uint64_t digits)
{
    char digits_buffer[100];
    int size = 0;
    int i = 0;
    if(digits == 0){
        buffer[position++] = 0 + 48;
        size++;
    }
    else{
        while(digits)
        {
            uint8_t bit = digits & 1;
            digits_buffer[size++] = bit + 48;
            digits = digits >> 1;
        }
        for (i = size-1; i >= 0; i--) {
            buffer[position++] = digits_buffer[i];
        }
    }
    return size;
}

static int decimal_to_string(char *buffer, int position, int64_t digits)
{
    int size = 0;

    if (digits < 0) {
        digits = -digits;
        buffer[position++] = '-';
        size = 1;
    }

    size += udecimal_to_string(buffer, position, (uint64_t)digits);
    return size;
}

static void write_console(const char *buffer, int size)
{
    int i = 0;

    for (i = 0; i < size; i++) {
        uart_write(buffer[i]);
    }
}


int printk(const char *format, ...)
{
    char buffer[1024];
    int buffer_size = 0;
    int64_t integer = 0;
    char *string = 0;
    int i = 0;
    va_list args;

    va_start(args, format);

    for (i = 0; format[i] != '\0'; i++) {
        if (format[i] != '%') {
            buffer[buffer_size++] = format[i];
        }
        else {
            switch (format[++i]) {
                case 'x':
                    integer = va_arg(args, int64_t);
                    buffer_size += hex_to_string(buffer, buffer_size, (uint64_t)integer);
                    break;

                case 'u':
                    integer = va_arg(args, int64_t);
                    buffer_size += udecimal_to_string(buffer, buffer_size, (uint64_t)integer);
                    break;
                
                case 'b':
                    integer = va_arg(args, int64_t);
                    buffer_size += udecimal_to_binary(buffer, buffer_size, (uint64_t)integer);
                    break;
                case 'd':
                    integer = va_arg(args, int32_t);
                    /*https://stackoverflow.com/questions/57748347/why-can-i-import-negative-numbers-as-int-but-not-as-long-int-or-other-larger-da*/
                    // integer = va_arg(args, int64_t); /*would be useful when int64 that time suffix "L" at the end to the data*/
                    buffer_size += decimal_to_string(buffer, buffer_size, integer);
                    break;

                case 's':
                    string = va_arg(args, char*);
                    buffer_size += read_string(buffer, buffer_size, string);
                    break;

                default:
                    buffer[buffer_size++] = '%';
                    i--;
            }
        }
    }

    write_console(buffer, buffer_size);
    va_end(args);

    return buffer_size;
}