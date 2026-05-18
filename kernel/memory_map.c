#include "memory_map.h"
#include "vga.h"

#include <stdint.h>

/* ============================================================
 * MULTIBOOT2 STRUCTURES
 * ============================================================ */

typedef struct {
    uint32_t total_size;
    uint32_t reserved;
} multiboot_info_t;

typedef struct {
    uint32_t type;
    uint32_t size;
} multiboot_tag_t;

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} multiboot_mmap_entry_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    multiboot_mmap_entry_t entries[];
} multiboot_tag_mmap_t;

/* ============================================================
 * CONSTANTS
 * ============================================================ */

#define MULTIBOOT_TAG_TYPE_END   0
#define MULTIBOOT_TAG_TYPE_MMAP  6

/* ============================================================
 * PRINT MEMORY MAP
 * ============================================================ */

void print_memory_map(uint64_t addr)
{
    vga_puts("ENTERED MEMORY MAP FUNCTION\n");

    multiboot_tag_t *tag =
        (multiboot_tag_t *)(addr + 8);

    int counter = 0;

    while (1)
    {
        vga_puts("TAG FOUND -> ");

        vga_putdec(tag->type);

        vga_putchar('\n');

        if (tag->type == MULTIBOOT_TAG_TYPE_END)
        {
            vga_puts("END TAG\n");
            break;
        }

        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP)
        {
            vga_puts("FOUND MMAP TAG\n");

            multiboot_tag_mmap_t *mmap_tag =
                (multiboot_tag_mmap_t *)tag;

            multiboot_mmap_entry_t *entry =
                mmap_tag->entries;

            while ((uint8_t *)entry <
                   ((uint8_t *)mmap_tag + mmap_tag->size))
            {
                vga_puts("BASE: ");
                vga_puthex(entry->addr);

                vga_puts(" | LENGTH: ");
                vga_puthex(entry->len);

                vga_puts(" | TYPE: ");

                vga_putdec(entry->type);

                vga_putchar('\n');

                entry = (multiboot_mmap_entry_t *)
                    ((uint8_t *)entry +
                     mmap_tag->entry_size);
            }
        }

        tag = (multiboot_tag_t *)
            ((uint8_t *)tag +
             ((tag->size + 7) & ~7));

        counter++;

        if (counter > 50)
        {
            vga_puts("TOO MANY TAGS -> BREAK\n");
            break;
        }
    }
}
