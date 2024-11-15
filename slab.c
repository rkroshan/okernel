#include "slab.h"
#include "assert.h"

extern page_t *get_free_page(void);

/**
 * @brief macro to locate kmem_buf_ctl after slab
 *
 */
#define slab_bufctl(slabp) ((kmem_bufctl_t *)(((slab_t *)slabp) + 1))
#define BUFCTL_END UINT32_MAX
/**
 * @brief intialisation function for
 * kmem_buf_ctl per slab
 * it is simple every array index will point to next index
 * and last index will point to macro BUFCTL_END
 */
void initialize_kmem_buf_ctl(kmem_bufctl_t *bufctl, uint64_t array_size) {
  for (uint64_t idx = 0; idx < array_size - 1; idx++) {
    bufctl[idx] = idx + 1;
  }
  // update last with BUFCTL_END
  bufctl[array_size - 1] = BUFCTL_END;
}

/**
 * @brief first kmem_cache object
 * statically defined this will act as slab for
 * another kmem_cache_t struct object allocation
 *
 */

static kmem_cache_t cache_cache;
static kmem_cache_t *global_cache_p = NULL;

/**
 * @brief allocate and setup a slab
 * to be added in slabs_empty of a cache
 * @param cache pointer to which this slab is part of
 */
static slab_t *alloc_slab(kmem_cache_t *cache) {
  /*get a page from buddy allocator*/
  page_t *page = get_free_page();
  /*this page struct is universal for this page
  set this to know that it is oned by slab now
  not buddy allocator*/
  page->page_owner = OWNER_SLAB;
  page->owner_kmem_cache_addr = cache;

  /*setup slab inside the page at initial*/
  slab_t *slab = (slab_t *)page->start_addr;
  slab->free = 0;
  slab->num_alloc_objects = 0;
  slab->next = NULL;

  /*setup kmem_buf_ctl after slab allocation*/
  uint64_t buf_ctl_start = (uint64_t)slab + sizeof(slab_t);
  /*objsize*x + 4*x = total_size
  so, x = total_size/(4+objsize)
  x = num of objects that can be allocated
  */
  uint64_t buf_ctl_array_size =
      (((uint64_t)slab + get_page_size()) - buf_ctl_start) /
      (cache->objsize + sizeof(kmem_bufctl_t));
  /*so smem from where actual object allocation will start will be after
   * kmem_buf_ctl*/
  slab->smem = (buf_ctl_start + buf_ctl_array_size * sizeof(kmem_bufctl_t));
  /*now need to initialise the kmem_buf_ctl*/
  initialize_kmem_buf_ctl((kmem_bufctl_t *)slab_bufctl(slab),
                          buf_ctl_array_size);

  return slab;
}

/**
 * @brief internal function to create cache for given size
 *
 */
static kmem_cache_t *alloc_cache(size_t size) {
  assert(size != 0U); /*this should not happen*/

  /*need to get memory for kmem_cache_t struct*/
  /*this will definitly return since we have already defined it during bootup*/
  kmem_cache_t *cache = (kmem_cache_t *)kmem_cache_alloc(sizeof(kmem_cache_t));
  cache->next = NULL;
  cache->objsize = size;
  cache->slabs_full = NULL;
  cache->slabs_partial = NULL;
  cache->slabs_empty = alloc_slab(cache);
  return cache;
}

/**
 * @brief internal function to get a cache for given size if
 * doesn't exist create one
 *
 */
static kmem_cache_t *get_size_cache(size_t size) {
  /*loop in the linked list to see if we already have a cache for this size*/
  kmem_cache_t *current_cache = global_cache_p;
  kmem_cache_t *found_cache = NULL;
  do {
    if (current_cache->objsize == size) {
      found_cache = current_cache;
      break; /*we found one*/
    }
    current_cache = current_cache->next;
  } while (current_cache != global_cache_p);

  if (found_cache != NULL) {
    return found_cache;
  } else {
    /*need to create cache for this size and add it into global cache list*/
    found_cache = alloc_cache(size);
    found_cache->next = global_cache_p;
    global_cache_p = found_cache;
    return found_cache;
  }
}

/**
 * @brief internal function to allocate memory from partial slab
 *
 */

static void *alloc_slab_partial_mem(kmem_cache_t *cache) {
  slab_t *current_partial_slab = cache->slabs_partial;
  void *addr = (void *)(current_partial_slab->smem +
                        cache->objsize * current_partial_slab->free);
  /*update the free index to value it holds in that index in bufctl array*/
  kmem_bufctl_t *bufctl_array = slab_bufctl(current_partial_slab);
  current_partial_slab->free = bufctl_array[current_partial_slab->free];
  current_partial_slab->num_alloc_objects++;

  /*now check if slab got full, then migrate it to slab full*/
  if (current_partial_slab->free == BUFCTL_END) {
    /*put the next partial slab to cache slab_partial*/
    cache->slabs_partial = current_partial_slab->next;
    /*put this partial slab which got full into slab full at top*/
    current_partial_slab->next = cache->slabs_full;
    cache->slabs_full = current_partial_slab;
  }

  return addr;
}

/**
 * @brief internal function to get memory allocated for slab
 * - first it will check in partial_slab, if no partial slab put an empty slab
 * into partial and allocate the memory, also update the buf_ctl
 * - if empty_slab is empty, alloc a new slab
 * - after allocation if partial slab got full, put it in slab_full
 */
static void *alloc_slab_mem(kmem_cache_t *cache) {
  /*look into partial slab if memory is there*/
  if (cache->slabs_partial != NULL) {
    return alloc_slab_partial_mem(cache);
  } else {
    /*get one slab from empty_slab and if empty is also null alloc one slab,
    then put it into slab partial then allocate the memory required*/
    if (cache->slabs_empty == NULL) {
      cache->slabs_empty = alloc_slab(cache);
    }

    /*put the empty_slab in partial slab*/
    slab_t *empty_slab = cache->slabs_empty;
    cache->slabs_empty = empty_slab->next;
    empty_slab->next = cache->slabs_partial;
    cache->slabs_partial = empty_slab;

    // Now allocate the memory
    return alloc_slab_partial_mem(cache);
  }
}

/**
 * @brief function to allocate
 * memory from slab allocator based on size
 *
 */
void *kmem_cache_alloc(size_t size) {
  /*don't call it before kmem_cache_boot_init*/
  assert(global_cache_p != NULL);
  /*sanity check if size is actually less than page_size*/
  assert(size < get_page_size());

  /*get cache for the size*/
  kmem_cache_t *cache = get_size_cache(size);
  /*allocate the memory from the slab*/
  return alloc_slab_mem(cache);
}

/**
 * @brief function to free
 * memory from slab and cache
 * based on cache_addr
 */
void kmem_cache_free(void *ptr, page_t *page) {
  /**amzing thing is page start_aadr will point to the slab who own this address
   * :) */
  slab_t *slab = (slab_t *)page->start_addr;
  kmem_cache_t *cache = (kmem_cache_t *)page->owner_kmem_cache_addr;
  /*need to check ptr is greater than smem or not, else we can't deallocate it*/
  if ((uint64_t)ptr < slab->smem) {
    return;
  }

  uint64_t ptr_idx = ((uint64_t)ptr - slab->smem) / cache->objsize;
  /*we need to index to update free in a way that now
  it will point to this index but this index will contain current free indx*/
  kmem_bufctl_t current_indx = slab->free;
  slab->free = ptr_idx;
  kmem_bufctl_t *bufctl = slab_bufctl(slab);
  bufctl[ptr_idx] = current_indx;

  // dec the num of objects
  slab->num_alloc_objects--;
}

/**
 * @brief function to initalisr first
 * kmem_cache object
 *
 */

void kmem_cache_boot_init(void) {
  // initialise cache to be use of kmem_cache_t object
  global_cache_p = &cache_cache;
  cache_cache.next = &cache_cache; /*circular cache list*/
  cache_cache.objsize = sizeof(kmem_cache_t);
  cache_cache.slabs_full = NULL;
  cache_cache.slabs_partial = NULL;
  cache_cache.slabs_empty = alloc_slab(&cache_cache);
}
