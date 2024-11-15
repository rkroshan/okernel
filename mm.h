#ifndef __MM_H__
#define __MM_H__

#include <stddef.h>
#include <stdint.h>

/**
 * @brief max order of pages that can be allocated
 *
 */
#define MAX_ORDER 11

/**
 * @brief minimum size of allocation in bytes
 *
 */
#define MIN_ALLOC_SIZE_IN_BYTES 32

/**
 * @brief enum to define total zones
 *
 */
typedef enum { ZONE_NOHEAP = 0, ZONE_HEAP, ZONES_COUNT } zone_enum_t;

/**
 * @brief enum for page owner information
 * when a page will be allocated we need to know who allocates it
 * buddy or slab
 * and if slab then what is the address for the kmem_cache
 * this will help in Kfree very quickly
 */
typedef enum { OWNER_BUDDY = 0, OWNER_SLAB, OWNER_COUNT } page_owner_enum_t;

/**
 * @brief structure to store per order information
 *
 */
typedef struct freeblock {
  struct freeblock *next; /*will store free blocks information*/
} FreeBlock_t;

typedef struct freearea {
  FreeBlock_t *freeblocks_list; /*will store list of free blocks*/
  uint64_t *bitmap;
  /*this bitmap is per pair of blocks if no block is allocated mask is 0;
  if a block is allocated from the pair the buddy will stay in the free_list and
  mask will be 1, but if second buddy also get allocated mask will get 0*/
} FreeArea_t;

/**
 * @brief zone struct to define attributes avialable per zone
 *
 */
typedef struct zone {
  uint8_t zone_id;     /*zone_id will be one of zone_enum_t*/
  uint8_t allocatable; /*defines if we can use this zone for heap allocation or
                          it is readonly*/
  uint8_t padding[6];
  uint64_t start_addr;        /*start addr for this contiguous memory zone*/
  size_t size;                /*max size of this contiguous memory*/
  FreeArea_t area[MAX_ORDER]; /*FreeArea struct per order to store free pages
                                 information and bitmap for those buddies*/
} zone_t;

/**
 * @brief struct page for per page information
 * size of page will always be PAGE_SIZE
 */
typedef struct page {
  uint8_t order;   /*this is necessary because during buddy deallocation we need
                      order information to find the buddy*/
  uint8_t zone_id; /*helpful while deallocating buddies else need to loop in
                      each zone to find where this address will land*/
  uint8_t page_owner; /*should be an page_owner_enum_t*/
  uint8_t padding[5]; /*will remove it later if need for something else*/
  void *owner_kmem_cache_addr; /*store the kmem_cache struct address to quickly
                                  find the cache which owns it*/
  uint64_t start_addr;         /*page start address*/
} page_t;

/**
 * @brief kmalloc_aligned to alloc memory align to alignment
 *
 */
void *kmalloc_aligned(size_t size, size_t alignment);

/**
 * @brief function to alloc memory
 * kmalloc: based on size it will decide from where to take the memory
 * if size > PAGE_SIZE buddy_alloc will be use
 * else kmem_cache_alloc will be use
 * Metadata : will be calulated first by getting the pfn for the address then
 * get the struct page and then check for who owns that page based on which
 * it will be dealloc by buddy or slab
 */
void *kmalloc(size_t size);
/**
 * @brief function to free memory
 * Kfree: first we will try to get the pfn for that addr
 * the get to know who owns that page
 * then call appropritae slab or buddy free
 */
void kfree(void *ptr);

/**
 * @brief return page index from the address
 * make sure that addr is page aligned
 * every page in whole memory of the system has an index
 */
uint64_t get_page_indx(uint64_t addr);
/**
 * @brief get zone information
 *
 */
zone_t *get_zone_info(uint8_t index);
/**
 * @brief get struct page from the page list
 * based on index
 * @param index of the page
 */
page_t *get_page_struct(uint64_t index);
/**
 * @brief function to get total memory of the system
 *
 */
uint64_t get_total_memory_in_bytes(void);
/**
 * @brief function to get page size for the system
 *
 */
uint64_t get_page_size(void);
/**
 * @brief function to get total number of pages available in the system in that
 * zone
 *
 */
uint64_t get_total_pages(void);
/**
 * @brief boot mem intialisation
 * necessary to allocate memory for structures use to manage memory
 *
 */
void boot_mem_init(void);

#endif