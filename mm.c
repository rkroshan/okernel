#include "mm.h"
#include "board.h"
#include "util.h"
#include "assert.h"
#include "buddy_alloc.h"
#include "slab.h"
/**
 * @brief this will the start of heap addr before 
 * which we have text, data and static stack allocation
 * defined in linker
 */
extern uint64_t heap_start;

/**
 * @brief this will define the zone heap start after we allocate the memory we needed for
 * pre intialisation structures
 * should it be from the zone itself? I don't think that is necessary since kernel can access any page 
 * 
 */
static uint64_t pre_init_heap_addr;
static uint64_t pre_init_num_of_pages = 0;

static zone_t zones[ZONES_COUNT];
static uint64_t struct_pages_list_start;

/**
 * @brief alloc memory for bitmap inside FreeArea_t struct per order per zone
 * bitmap will represent whole memory bitmap for that zone contiguous memory
 * it doesn't depend on how much memory can be allocated by buddy allocator at once contiguously
 * 
 */
static void pre_alloc_bitmap_per_freearea_struct(zone_t* zone)
{
    if(!zone->allocatable){
        //not allocatable why to waste memory
        return NULL;
    }

    /*calculate total bitmap size requirement
    this is independent of zone it is for whole available memory*/
    uint64_t total_bitmap_size = 0;
    for(uint8_t order=0;order < MAX_ORDER;order++)
    {
        // np = (total_pages/(1<<order))/2
        //total bits = ((np + bit_in_uint64 -1)/bit_int_uint64) * sizeof(uint64_t)
        uint64_t bitmap_size = (((((get_total_pages()/BIT(order))/2) + BITS_PER_UINT64 -1) / BITS_PER_UINT64) * sizeof(uint64_t));
        /*set the bitmap addr in freearea order*/
        zone->area[order].bitmap = pre_init_heap_addr + total_bitmap_size;
        total_bitmap_size += bitmap_size;
        memset(zone->area[order].bitmap,0x0,bitmap_size); /*clean it to show that no block pair is available at that bit*/
    }
        pre_init_heap_addr += total_bitmap_size; /*update pre_init_heap address*/

    printk("Total bitmap size required for zone:%d = %u\n", zone->zone_id, total_bitmap_size);
}

/**
 * @brief allocate memory for struct page for whole memory total pages
 * 
 */
static void pre_alloc_pages_struct(void)
{
    //alignment check already taken care in structure intialisation
    //make sure to check for alignment warning -Wpadded
    uint64_t total_size = sizeof(page_t)*get_total_pages();
    struct_pages_list_start = pre_init_heap_addr;
    pre_init_heap_addr += total_size;
}

/**
 * @brief get struct page from the page list
 * based on index
 * @param index of the page
 */
page_t* get_page_struct(uint64_t index)
{
    //typecast the pages_list start to struct page and increment by index and then return the typcasted address
    return (page_t*)((page_t*)(struct_pages_list_start) + index);
}

/**
 * @brief function to get total memory of the system
 * 
 */
uint64_t get_total_memory_in_bytes(void)
{
    return RAM_SIZE;
}

/**
 * @brief return page index from the address
 * make sure that addr is page aligned
 * every page in whole memory of the system has an index
 * 
 * it is actually pfn also
 */
uint64_t get_page_indx(uint64_t addr)
{
    return (addr >> get_page_size());
}

/**
 * @brief function to get page size for the system
 * 
 */
uint64_t get_page_size(void)
{
    return PAGE_SIZE;
}

/**
 * @brief function to get total number of pages available in the system in that zone
 * 
 */
uint64_t get_total_pages(void)
{
    return (get_total_memory_in_bytes()/get_page_size());
}

/**
 * @brief initialise zone objects
 * 
 */
static void zone_init(void)
{
    //zone 0
    // this is our entire static data stack and code memory region
    // before heap_start 
    zones[ZONE_NOHEAP].allocatable = 0U; /*NO*/
    zones[ZONE_NOHEAP].zone_id = ZONE_NOHEAP;
    zones[ZONE_NOHEAP].start_addr = RAM_START;
    zones[ZONE_NOHEAP].size = (heap_start - zones[ZONE_NOHEAP].start_addr);
    memset(&zones[ZONE_NOHEAP].area, 0x0, sizeof(FreeArea_t)*MAX_ORDER); /*clean up FreaArea struct memory*/

    // TODO: need to split zones in high and low heap (maybe like user and kernel as well ?)!!!
    //currently whole heap memory as one
    zones[ZONE_HEAP].allocatable = 1U; /*YESSSS*/
    zones[ZONE_HEAP].zone_id = ZONE_HEAP;
    zones[ZONE_HEAP].start_addr = heap_start;
    zones[ZONE_HEAP].size = get_total_memory_in_bytes() - zones[ZONE_HEAP].start_addr;
    memset(&zones[ZONE_HEAP].area, 0x0, sizeof(FreeArea_t)*MAX_ORDER); /*clean up FreaArea struct memory*/
    /*get memory for bitmap*/
    pre_alloc_bitmap_per_freearea_struct(&zones[ZONE_HEAP]);

    //print zone information
    for(uint8_t idx=0;idx<ZONES_COUNT;idx++){
        printk("zone: %d allocatable:%d start:%x end:%x\n",zones[idx].zone_id, zones[idx].allocatable, zones[idx].start_addr, zones[idx].size);
    }

}

/**
 * @brief get zone information
 * 
 */
zone_t* get_zone_info(uint8_t index)
{
    if(index >= ZONES_COUNT)
    {
        return NULL;
    }

    return &zones[index];
}

/**
 * @brief function to alloc memory
 * kmalloc: based on size it will decide from where to take the memory
 * if size > PAGE_SIZE buddy_alloc will be use
 * else kmem_cache_alloc will be use
 * Metadata : will be calulated first by getting the pfn for the address then 
 * get the struct page and then check for who owns that page based on which
 * it will be dealloc by buddy or slab
 */
void* kmalloc(size_t size)
{
    /*why to waste time*/
    if(size == 0U){
        return NULL;
    }

    if(size >= get_page_size()){
        //going to use buddy allocator
        /*need to align the size to nearest page*/
        size_t aligned_size = _alignto(size,get_page_size());
        uint8_t order = __builtin_ctz(aligned_size) - __builtin_ctz(get_page_size()); /*trailing zeros == powerof2*/
        page_t* page = get_free_pages(order);
        return (void*)page->start_addr;
    }
    else{
        //need to go with slab allocator
        /*need to align to 4bytes*/
        size_t aligned_size = _alignto(size,sizeof(uint32_t));
        return kmem_cache_alloc(size);
    }
}

/**
 * @brief function to free memory
 * Kfree: first we will try to get the pfn for that addr 
 * the get to know who owns that page
 * then call appropritae slab or buddy free
 */
void kfree(void* ptr)
{
    page_t* page = get_page_struct(get_page_indx((uint64_t)ptr));
    if(page->page_owner == OWNER_BUDDY){
        //page is owned by buddy allocator
        get_free_pages(page->order);
    }
    else if(page->page_owner = OWNER_SLAB){
        assert(page->owner_kmem_cache_addr != NULL); /*only slab_alloc should set this then how come?*/
        //page is owned by slab allocator and by which cache
        kmem_cache_free(ptr,page);
    }
    else{
        //ignore
    }
}

/**
 * @brief boot mem intialisation
 * necessary to allocate memory for structures use to manage memory
 * 
 */
void boot_mem_init(void)
{
    //initialise pre_init_heap_addr
    pre_init_heap_addr = heap_start;

    zone_init(); /*bitmap pre_alloc done inside here*/
    pre_alloc_pages_struct();

    //update the memory left in zone_heap from next page
    zones[ZONE_HEAP].start_addr = _alignto(pre_init_heap_addr,get_page_size());
    zones[ZONE_HEAP].size = get_total_memory_in_bytes() - zones[ZONE_HEAP].start_addr;

    //at last initialise the buddy_allocator
    buddy_heap_init();

    //kmem cache init
    kmem_cache_boot_init();
}