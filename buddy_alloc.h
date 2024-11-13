#ifndef __BUDDY_ALLOC_H__
#define __BUDDY_ALLOC_H__

#include <stddef.h>
#include <stdint.h>
#include "mm.h"

/**
 * @brief init the heap for information about zone memory regions
 * from where to pick the heap and it's size
 *
 * start placing the blocks in highest order if still memory left place then 
 * in lower memory order
 * bitmap memory is already carvedout just need to place the blocks in appropriate
 * freelists
 *
 * memory region should be PAGE_SIZE aligned else initialisation will fail with panic
 */
void buddy_heap_init(void);

/**
 * @brief get a free page
 * @return page struct pointer
 */
page_t* get_free_page(void);

/**
 * @brief get free pages, count  = 2^(order)
 * make sure order is less than (max order -1)
 * @return page struct pointer
 */
page_t* get_free_pages(uint8_t order);

/**
 * @brief free the page based on order
 * TODO: will use some struct to have this information of order
 * order is assumed to be zero
 * @param page struct pointer
 */
void free_page(page_t* page);

/**
 * @brief free the page based on order
 * TODO: should we also check if page lies in page_struct region?
 * @param page struct pointer
 * @param order of the page
 */
void free_pages(page_t* page, uint8_t order);

#endif