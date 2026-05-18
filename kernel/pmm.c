#include "pmm.h"
#include "vga.h"
#include <stddef.h>
#include <stdint.h>

/* ─────────────────────────────────────────────────────
 * Bitmap lives in BSS.  Supports up to 4 GB of RAM
 * (4 GB / 4 KB pages = 1 M frames → 128 KB bitmap).
 * ───────────────────────────────────────────────────── */
#define MAX_FRAMES   (1024 * 1024)          /* 1M frames  */
static uint8_t  bitmap[MAX_FRAMES / 8];
static uint64_t total_frames = 0;
static uint64_t free_frames  = 0;

/* ─── helpers ─────────────────────────────────────── */
static inline void bitmap_set(uint64_t frame)   { bitmap[frame / 8] |=  (1u << (frame % 8)); }
static inline void bitmap_clear(uint64_t frame) { bitmap[frame / 8] &= ~(1u << (frame % 8)); }
static inline int  bitmap_test(uint64_t frame)  { return (bitmap[frame / 8] >> (frame % 8)) & 1; }

/* ─── init ────────────────────────────────────────── */
void pmm_init(uint64_t mmap_addr, uint32_t mmap_len,
              uint64_t kernel_start, uint64_t kernel_end)
{
    /* 1. Mark everything as USED first */
    for (uint64_t i = 0; i < MAX_FRAMES / 8; i++) bitmap[i] = 0xFF;

    /* 2. Walk Multiboot2 memory map, mark available regions FREE */
    uint64_t offset = 0;
    while (offset < mmap_len) {
        mmap_entry_t *e = (mmap_entry_t *)(mmap_addr + offset);
        if (e->type == 1) {                 /* available */
            uint64_t start_frame = (e->base_addr + PAGE_SIZE - 1) / PAGE_SIZE;
            uint64_t end_frame   = (e->base_addr + e->length) / PAGE_SIZE;
            for (uint64_t f = start_frame; f < end_frame; f++) {
                bitmap_clear(f);
                free_frames++;
            }
            if (end_frame > total_frames) total_frames = end_frame;
        }
        offset += sizeof(mmap_entry_t);     /* fixed-size entries for simplicity */
    }

    /* 3. Re-mark first 1 MB as USED (BIOS/VGA/bootloader) */
    pmm_mark_used(0, 256);                  /* 256 * 4 KB = 1 MB */

    /* 4. Re-mark kernel image pages as USED */
    uint64_t ks = kernel_start / PAGE_SIZE;
    uint64_t ke = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    pmm_mark_used(ks * PAGE_SIZE, ke - ks);

    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("  [ OK ] PMM initialised — ");
    vga_putdec(free_frames * 4 / 1024);
    vga_puts(" MB free\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void pmm_mark_used(uint64_t phys_addr, size_t page_count)
{
    uint64_t frame = phys_addr / PAGE_SIZE;
    for (size_t i = 0; i < page_count; i++) {
        if (!bitmap_test(frame + i)) { bitmap_set(frame + i); free_frames--; }
    }
}

void pmm_mark_free(uint64_t phys_addr, size_t page_count)
{
    uint64_t frame = phys_addr / PAGE_SIZE;
    for (size_t i = 0; i < page_count; i++) {
        if (bitmap_test(frame + i)) { bitmap_clear(frame + i); free_frames++; }
    }
}

uint64_t pmm_alloc_page(void)
{
    for (uint64_t i = 0; i < total_frames; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            free_frames--;
            return i * PAGE_SIZE;
        }
    }
    return 0;   /* Out of memory */
}

void pmm_free_page(uint64_t phys_addr)
{
    uint64_t frame = phys_addr / PAGE_SIZE;
    if (bitmap_test(frame)) { bitmap_clear(frame); free_frames++; }
}

/* ─── Tensor Allocator — contiguous pages ─────────── */
uint64_t pmm_alloc_contiguous(size_t page_count)
{
    uint64_t run_start = 0;
    size_t   run_len   = 0;
    for (uint64_t i = 0; i < total_frames; i++) {
        if (!bitmap_test(i)) {
            if (run_len == 0) run_start = i;
            run_len++;
            if (run_len == page_count) {
                for (size_t j = 0; j < page_count; j++) bitmap_set(run_start + j);
                free_frames -= page_count;
                return run_start * PAGE_SIZE;
            }
        } else {
            run_len = 0;
        }
    }
    return 0;   /* Not enough contiguous memory */
}

uint64_t pmm_get_total_pages(void) { return total_frames; }
uint64_t pmm_get_free_pages(void)  { return free_frames; }

void pmm_dump_stats(void)
{
    vga_puts("PMM: total=");
    vga_putdec(total_frames);
    vga_puts(" free=");
    vga_putdec(free_frames);
    vga_puts(" used=");
    vga_putdec(total_frames - free_frames);
    vga_puts("\n");
}

void pmm_debug_set_free(uint64_t pages)
{
    free_frames = pages;
}
