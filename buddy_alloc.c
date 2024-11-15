#include "buddy_alloc.h"
#include "assert.h"
#include "board.h"
#include "mm.h"
#include "util.h"

/*Limitation of buddy allocator is you cannot allocate memory
 * more than 2^(MAX_ORDER-1)*PAGE_SIZE at once*/
#define ORDER_SIZE(n) (PAGE_SIZE * (1 << n))
// Bitmap index and position macros
#define BITMAP_INDEX(block) ((block) / BITS_PER_UINT64)
#define BIT_POSITION(block) ((block) % BITS_PER_UINT64)

/**
 * @brief when the block is taken from a freearea
 * or given back
 * mark the bitmap by toggling the block pair bit
 *
 */
static void buddy_toggle_bitmap(uint64_t index, uint8_t order,
                                FreeArea_t *area) {
  /*block_index in that order = index of page / (order)
  pair_indx = block_idx/2
  bitmap_idx = BITMAP_INDEX(pair_indx)
  bit_position = BIT_POSITION(pair_indx)*/
  uint64_t pair_indx = (index / BIT(1 + order));
  area->bitmap[BITMAP_INDEX(pair_indx)] ^= (1UL << BIT_POSITION(pair_indx));
}

/**
 * @brief get the bitmap for the indez and order
 *
 */
static uint8_t buddy_get_bitmap(uint64_t index, uint8_t order,
                                FreeArea_t *area) {
  /*block_index in that order = index of page / (order)
  pair_indx = block_idx/2
  bitmap_idx = BITMAP_INDEX(pair_indx)
  bit_position = BIT_POSITION(pair_indx)*/
  uint64_t pair_indx = (index / BIT(1 + order));
  return (uint8_t)((area->bitmap[BITMAP_INDEX(pair_indx)] &
                    (1UL << BIT_POSITION(pair_indx))) >>
                   BIT_POSITION(pair_indx));
}

/**
 * @brief init the heap for information about zone memory regions
 * from where to pick the heap and it's size
 *
 * start placing the blocks in highest order if still memory left place then
 * in lower memory order
 * bitmap memory is already carvedout just need to place the blocks in
 * appropriate freelists
 *
 * memory region should be PAGE_SIZE aligned else initialisation will fail with
 * panic
 * @return error code for region addition to heap
 */
void buddy_heap_init() {
  /*loop for all zone and if allocatable then start adding blocks in free list*/
  for (uint8_t zone_idx = 0; zone_idx < ZONES_COUNT; zone_idx++) {
    zone_t *zone = get_zone_info(zone_idx);
    if (!zone->allocatable) { /*it is not for heap*/
      continue;
    }

    printk_debug("zone_start_addr : %x size : %x\n", zone->start_addr,
                 zone->size);
    /*memory region should be PAGE_SIZE aligned else initialisation will fail
     * with panic*/
    if (!_is_align(zone->start_addr, get_page_size())) {
      fatal("zone start addr is not aligned\n");
    }

    uint64_t available_memory_start = zone->start_addr;
    uint64_t available_memory_size = zone->size;
    for (uint8_t order = MAX_ORDER - 1; order < UINT8_MAX; order--) {
      /*get no of blocks possible to fit for current order*/
      uint64_t nblocks = available_memory_size / (BIT(order) * get_page_size());
      if (nblocks ==
          0) /*that means we can't fit a block in freearea for that order*/
      {
        continue;
      }
      /*if it is possible fill in the blocks in freearea list for that order*/
      for (uint64_t blk_idx = 0; blk_idx < nblocks; blk_idx++) {
        FreeBlock_t *current_blk =
            (FreeBlock_t *)(available_memory_start +
                            (blk_idx * BIT(order) * get_page_size()));
        current_blk->next = zone->area[order].freeblocks_list;
        zone->area[order].freeblocks_list = current_blk;
      }
      /*update the available memory*/
      available_memory_size -= (nblocks * BIT(order) * get_page_size());
      available_memory_start += (nblocks * BIT(order) * get_page_size());

      printk_debug("buddy_heap_init: order:%d nblocks:%u new_mem_start:%x "
                   "size_remaining:%x\n",
                   order, nblocks, available_memory_start,
                   available_memory_size);
    }
  }
}

/**
 * @brief actual function to alloc memory based on order
 * working:
 * - looks of freeblocks in order list, if not available
 * - split higher order blocks and remove it from their freeblock list and mark
 * bitmap idx as used
 * - push their buddy in their free block list of lower order and
 * - mark the bitmap idx for the pair as 1(used) by xoring the bitmap idx
 * - so next time if same oder allocation is requested then bitmap idx will be 0
 * indicating both blocks not available or free
 * - and remove it from freeblock list
 * @return struct page_t for the allocation
 */
static page_t *buddy_alloc(uint8_t order) {
  /*sanity check: if order is greater than MAX_ORDER-1*/
  if (order >= MAX_ORDER) {
    printk_debug("buddy_alloc: order : %u greater than MAX_ORDER-1: %u", order,
                 MAX_ORDER - 1);
    return NULL;
  }

  for (uint8_t zone_idx = 0; zone_idx < ZONES_COUNT; zone_idx++) {
    zone_t *zone = get_zone_info(zone_idx);
    if (!zone->allocatable) { /*it is not for heap*/
      continue;
    }

    for (uint8_t current_order = order; current_order < MAX_ORDER;
         current_order++) {
      if (zone->area[current_order].freeblocks_list != NULL) {
        /*found one block save it and remove it from free block list*/
        FreeBlock_t *block = zone->area[current_order].freeblocks_list;
        zone->area[current_order].freeblocks_list = block->next;
        /*also need to update the bitmap*/
        /*this is necessary because it help while merging
        if bitmap idx is 0 then we can't merge
        because one of the buddies still may be used
        if bitmap indx remain 1 while freeing it means other buddy is still in
        freearea list we can merge*/
        buddy_toggle_bitmap(get_page_indx((uint64_t)block), current_order,
                            &zone->area[current_order]);

        /*now need to check if we took block from high order then we need to
         * split the blocks*/
        while (current_order > order) {
          current_order--;
          /*buddy need to be pushed onto lower order freeblock list*/
          uint64_t buddy_address =
              (uint64_t)block ^
              (1UL << (current_order + _get_p2(get_page_size())));
          FreeBlock_t *buddy = (FreeBlock_t *)buddy_address;
          buddy->next = zone->area[current_order].freeblocks_list;
          zone->area[current_order].freeblocks_list = buddy;
          /*also update the bitmap*/
          buddy_toggle_bitmap(get_page_indx(buddy_address), current_order,
                              &zone->area[current_order]);
        }

        /*Now we need to fill in struct page for starting page and return it*/
        page_t *page = get_page_struct(get_page_indx((uint64_t)block));
        page->order = order;
        page->start_addr = (uint64_t)block;
        page->zone_id = zone_idx;
        page->page_owner = OWNER_BUDDY;
        page->owner_kmem_cache_addr = NULL;
        return page;
      }
    }
  }

  printk_error("Buddy_alloc: No block found!!!\n");
  return NULL; /*didn't found any free block :(, maybe need to increase
                  MAX_ORDER for now?*/
}

/**
 * @brief actual function to free the memory and return back into freeblocks of
 * that order if possible will coalesce it if both buddies available
 */
static void buddy_free(page_t *page, uint8_t order) {
  /*check if page is NULL*/
  if (!page)
    return;

  /*assert that page order is matching with order given */
  assert(page->order == order);

  uint64_t address = page->start_addr;         /*page address to free*/
  zone_t *zone = get_zone_info(page->zone_id); /*zone in which page is present*/

  assert((address >= zone->start_addr) &&
         (address < (zone->start_addr + zone->size)));
  /*will try add the block into free block or if possible try to coalesce it*/
  uint8_t current_order = page->order;
  while (current_order < MAX_ORDER) {
    /*let check if it's buddy is also available via bitmap*/
    /*current bitmap value if 1 it means buddy is free else cannot coalesce*/
    uint8_t bitset = buddy_get_bitmap(get_page_indx(address), current_order,
                                      &zone->area[current_order]) &
                     1U;
    /*update the bitmap*/
    buddy_toggle_bitmap(get_page_indx(address), current_order,
                        &zone->area[current_order]);
    if (bitset) {
      /*that means another block is free we can merge this block with that*/
      /*remove the buddy from freeblock list of current order*/
      FreeBlock_t *current_block = zone->area[current_order].freeblocks_list;
      FreeBlock_t *prev_block = NULL;
      uint64_t buddy_address = (uint64_t)address ^ (1 << current_order);
      FreeBlock_t *buddy = NULL;
      while (current_block != NULL) {
        if (current_block == (FreeBlock_t *)buddy_address) {
          printk_debug("buddy_free: Found it!!\n");
          /*remove it*/
          if (prev_block != NULL) {
            /*that means the block is somewhere in between*/
            prev_block->next = current_block->next;
          } else {
            /*it is at head*/
            current_block = current_block->next;
          }
          /*for reference*/
          buddy = (FreeBlock_t *)buddy_address;
          break;
        }
        prev_block = current_block;
        current_block = current_block->next;
      }
      if ((uint64_t)buddy != buddy_address) {
        /*2 possibities: either bit logic is wrong or
        race condition need to appropriately setup locks in smp */
        printk_critical("BUDDY_FREE: This should not happen!!");
      }

      /*if second block was free and first was already then start address should
      be first block address it is necessary because that's how we will be able
      to find the buddy in next order*/
      if (address > buddy_address) {
        address = buddy_address;
      }
    } else {
      /*means other buddy is busy allocated so cannot merge then
      add it to order FreeBlock */
      FreeBlock_t *buddy_block = (FreeBlock_t *)address;
      buddy_block->next = zone->area[current_order].freeblocks_list;
      zone->area[current_order].freeblocks_list = buddy_block;
      return; /*further order blocks also cannot be merged*/
    }
    current_order++;
  }

  // update page info that it is now not own by buddy
  // this will help prevent free pages not owned by buddy but earlier were owned
  page->page_owner = OWNER_COUNT;
}

/**
 * @brief get a free page
 * @return page struct pointer
 */
page_t *get_free_page(void) { return buddy_alloc(0); }

/**
 * @brief get free pages, count  = 2^(order)
 * make sure order is less than (max order -1)
 * @return page struct pointer
 */
page_t *get_free_pages(uint8_t order) { return buddy_alloc(order); }

/**
 * @brief free the page based on order
 * TODO: will use some struct to have this information of order
 * order is assumed to be zero
 * @param page struct pointer
 */
void free_page(page_t *page) { return buddy_free(page, 0); }

/**
 * @brief free the page based on order
 * TODO: should we also check if page lies in page_struct region?
 * @param order of the page
 */
void free_pages(page_t *page, uint8_t order) { return buddy_free(page, order); }
