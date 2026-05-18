#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

/* Physical Memory Manager — Bitmap Allocator
 * Each bit in the bitmap represents one 4KB physical page frame.
 * Bit = 1 → frame is USED; Bit = 0 → frame is FREE.
 */

#define PAGE_SIZE       4096
#define PAGES_PER_BYTE  8

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;          /* 1 = available, 2 = reserved, etc. */
} __attribute__((packed)) mmap_entry_t;

void  pmm_init(uint64_t mmap_addr, uint32_t mmap_len, uint64_t kernel_start, uint64_t kernel_end);
void  pmm_mark_used(uint64_t phys_addr, size_t page_count);
void  pmm_mark_free(uint64_t phys_addr, size_t page_count);
uint64_t pmm_alloc_page(void);
void  pmm_free_page(uint64_t phys_addr);
uint64_t pmm_alloc_contiguous(size_t page_count);   /* Tensor Allocator */
uint64_t pmm_get_total_pages(void);
uint64_t pmm_get_free_pages(void);
void  pmm_dump_stats(void);
void pmm_debug_set_free(uint64_t pages);

#endif /* PMM_H */
