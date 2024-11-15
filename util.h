#ifndef __UTIL_H__
#define __UTIL_H__

#include "printk.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Unsigned long integer with bit position n set
 */
#define BIT(n) (1UL << (n))
/**
 * @brief Unsigned integer with bit position n set
 */
#define UBIT(n) (1U << (n))
/**
 * @brief aligned of x to size in bytes mentioned
 * it only works if alignment to be power of 2
 */
#define _alignto(x, alignment_in_bytes)                                        \
  ((x + alignment_in_bytes - 1U) & ~(alignment_in_bytes - 1U))

/**
 * @brief align till that x not beyond that
 *
 */
#define _aligntill(x, alignment_in_bytes) (x & ~(alignment_in_bytes - 1U))

/**
 * @brief help macro to check if element is aligned to alignment
 *
 */
#define _is_align(x, alignment_in_bytes) ((x & (alignment_in_bytes - 1U)) == 0)

/**
 * @brief check if element is power of 2
 * zero is consider power of 2
 */
#define _is_power_of_two(x) ((x != 0) && (((x) & (x - 1)) == 0))

/**
 * @brief get power of 2
 *
 */
#define _get_p2(x) (63U - (__builtin_clzll((uint64_t)(x))))

/**
 * @brief align down to power of 2
 *
 */
#define _aligndown_p2(x, b) (((x) >> (b)) << (b))

/**
 * @brief align up to power of 2
 *
 */
#define _alignup_2(x, b) _aligndown_p2(((x) + BIT(b) - 1UL), b)

/**
 * @brief bits count in a uint64_t
 *
 */
#define BITS_PER_UINT64 (sizeof(uint64_t) * 8U)

/*memory operation functions*/
void memset(void *dst, uint8_t value, size_t size);
void memcpy(void *dst, void *src, unsigned int size);
void memmove(void *dst, void *src, unsigned int size);
int memcmp(void *src1, void *src2, unsigned int size);
uint64_t strlen(const char *string);

#endif