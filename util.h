#ifndef __UTIL_H__
#define __UTIL_H__

#include <stddef.h>

/**
 * @brief Unsigned long integer with bit position n set
 */
#define BIT(n) (1UL << (n))
/**
 * @brief Unsigned integer with bit position n set
 */
#define UBIT(n) (1U << (n))
/**
 * @brief aligned of address to size in bytes mentioned
 * it only works if alignment to be power of 2
 */
#define _alignto(addr,alignment_in_bytes) ((addr + alignment_in_bytes - 1U) & ~(alignment_in_bytes - 1U))

/**
 * @brief align till that addr not beyond that
 * 
 */
#define _aligntill(addr,alignment_in_bytes) (addr  & ~(alignment_in_bytes - 1U))

/**
 * @brief help macro to check if element is aligned to alignment
 * 
 */
 #define _is_align(addr, alignment_in_bytes) ((addr & (alignment_in_bytes - 1U)) == 0)

/**
 * @brief check if element is power of 2
 * zero is consider power of 2
 */
#define _is_power_of_two(addr)  ((addr != 0) && (((addr)&(addr-1)) == 0))

/**
 * @brief bits count in a uint64_t
 * 
 */
#define BITS_PER_UINT64 (sizeof(uint64_t) * 8U)

/*memory operation functions*/
void memset(void *dst, uint8_t value, size_t size);

#endif