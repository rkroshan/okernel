#ifndef __SLAB_H__
#define __SLAB_H__

#include <stdint.h>

/**
 * @brief free object position
 * in the slab page
 * 
 */
typedef uint32_t kmem_bufctl_t;

/**
 * @brief slab struct to hold pages
 * one slab contains one page
 * and next page will be in linked list
 * these pages will gets moved to partial or full slab list 
 * when their conditioned are met
 */
typedef struct slab{
    kmem_bufctl_t free; /*index to free object and next object location will be the data of the index*/
    uint64_t smem;/*this slab page start address*/
    uint64_t num_alloc_objects; /*gives the num of objects allocated from this slab*/
    struct slab* next;
    //kmem_buf_ctl array will be allocated here after slab descriptor, don't have pointer here to save memory :)
}slab_t;

/**
 * @brief kmem_cache structure to holds slabs
 * 
 */
typedef struct kmem_cache{
    struct kmem_cache* next; /*points to next kmem_cache*/
    slab_t* slabs_full; /*when slabs gets full*/
    slab_t* slabs_partial; /*when memory is allocatable from slab*/
    slab_t* slabs_empty; /*when whole page is empty*/
    uint64_t objsize; /*size of object this cache can allocate*/
    //TODO: spinlock
}kmem_cache_t;

/**
 * @brief function to free
 * memory from slab and cache
 * based on page
 */
void kmem_cache_free(void* ptr, page_t* page);

/**
 * @brief function to allocate
 * memory from slab allocator based on size
 * 
 */
void* kmem_cache_alloc(size_t size);

/**
 * @brief function to initalisr first
 * kmem_cache object
 * 
 */
void kmem_cache_boot_init(void);

#endif