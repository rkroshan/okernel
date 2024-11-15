#ifndef __PRINTK_H__
#define __PRINTK_H__

#include "errno.h"

/**
 * @brief print kernel logs
 *
 * @param log_level
 * @param format
 * @param ...
 * @return int
 */
int printk(log_level_e log_level, char *format, ...);

// Define macros for different log levels
#define printk_debug(format, ...) printk(DEBUG, format, ##__VA_ARGS__)
#define printk_info(format, ...) printk(INFO, format, ##__VA_ARGS__)
#define printk_warn(format, ...) printk(WARNING, format, ##__VA_ARGS__)
#define printk_error(format, ...) printk(ERROR, format, ##__VA_ARGS__)
#define printk_critical(format, ...) printk(CRITICAL, format, ##__VA_ARGS__)

#endif