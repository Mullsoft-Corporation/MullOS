/*
 * Physical Frame Allocator (Bitmap Architecture) & Page Table Paging System.
 * Manages Physical RAM blocks, Page Directories, Page Tables, and Fault Handling.
 */

#include "kernel.h"

extern void terminal_writestring(const char* data);
extern void kprintf_hex(uint32_t val);

#define FRAMES_PER_BYTE 8
#define PAGE_SIZE 4096

static uint32_t* pmm_bitmap = NULL;
static uint32_t pmm_max_frames = 0;
static uint32_t pmm_used_frames = 0;

/* External Symbols Defined in Linker Script */
extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

static inline void bitmap_set(uint32_t frame) {
    pmm_bitmap[frame / 32] |= (1 << (frame % 32));
}

static inline void bitmap_clear(uint32_t frame) {
    pmm_bitmap[frame / 32] &= ~(1 << (frame % 32));
}

static inline bool bitmap_test(uint32_t frame) {
    return (pmm_bitmap[frame / 32] & (1 << (frame % 32))) != 0;
}

static int32_t pmm_first_free_frame(void) {
    for (uint32_t i = 0; i < pmm_max_frames / 32; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFF) {
            for (int j = 0; j < 32; j++) {
                uint32_t bit = 1 << j;
                if (!(pmm_bitmap[i] & bit)) {
                    return i * 32 + j;
                }
            }
        }
    }
    return -1;
}

void pmm_init(uint32_t mem_size, uint32_t bitmap_address) {
    pmm_max_frames = mem_size / PAGE_SIZE;
    pmm_used_frames = pmm_max_frames;
    pmm_bitmap = (uint32_t*) bitmap_address;

    /* Mark all memory as occupied initially */
    for (uint32_t i = 0; i < pmm_max_frames / 32; i++) {
        pmm_bitmap[i] = 0xFFFFFFFF;
    }

    /* Free usable physical memory areas */
    for (uint32_t i = 0x100000; i < mem_size; i += PAGE_SIZE) {
        uint32_t frame = i / PAGE_SIZE;
        bitmap_clear(frame);
        pmm_used_frames--;
    }
}

void* pmm_alloc_frame(void) {
    if (pmm_used_frames >= pmm_max_frames) {
        return NULL; /* Out of Physical Memory */
    }

    int32_t frame = pmm_first_free_frame();
    if (frame == -1) {
        return NULL;
    }

    bitmap_set(frame);
    pmm_used_frames++;
    return (void*)(frame * PAGE_SIZE);
}

void pmm_free_frame(void* frame_addr) {
    uint32_t frame = (uint32_t)frame_addr / PAGE_SIZE;
    if (bitmap_test(frame)) {
        bitmap_clear(frame);
        pmm_used_frames--;
    }
}

/* Page Directory & Paging Configuration Definitions */
struct page_table_entry {
    uint32_t present  : 1;
    uint32_t rw       : 1;
    uint32_t user     : 1;
    uint32_t accessed : 1;
    uint32_t dirty    : 1;
    uint32_t unused   : 7;
    uint32_t frame    : 20;
};

struct page_table {
    struct page_table_entry pages[1024];
};

struct page_directory_entry {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t write_thru : 1;
    uint32_t cache_dis  : 1;
    uint32_t accessed   : 1;
    uint32_t reserved   : 1;
    uint32_t page_size  : 1;
    uint32_t unused     : 4;
    uint32_t pt_address : 20;
};

struct page_directory {
    struct page_directory_entry tables[1024];
};

static struct page_directory* current_directory = NULL;

void paging_init(void) {
    /* Allocate Identity Paging Page Directory */
    struct page_directory* dir = (struct page_directory*) pmm_alloc_frame();
    for (int i = 0; i < 1024; i++) {
        dir->tables[i].present = 0;
        dir->tables[i].rw = 1;
        dir->tables[i].user = 0;
    }

    /* Identity map the first 8MB of physical RAM */
    for (uint32_t i = 0; i < 0x800000; i += PAGE_SIZE) {
        uint32_t pd_index = i >> 22;
        uint32_t pt_index = (i >> 12) & 0x03FF;

        if (!dir->tables[pd_index].present) {
            struct page_table* pt = (struct page_table*) pmm_alloc_frame();
            dir->tables[pd_index].present = 1;
            dir->tables[pd_index].rw = 1;
            dir->tables[pd_index].pt_address = ((uint32_t)pt) >> 12;
        }

        struct page_table* pt = (struct page_table*)(dir->tables[pd_index].pt_address << 12);
        pt->pages[pt_index].present = 1;
        pt->pages[pt_index].rw = 1;
        pt->pages[pt_index].frame = i >> 12;
    }

    current_directory = dir;
    
    /* Load CR3 and Enable Paging Bit in CR0 */
    __asm__ volatile ("mov %0, %%cr3" : : "r"(current_directory));
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}
