/* ============================================================
 * AIOS — Kernel Main
 * ============================================================ */

#include "include/vga.h"
#include "include/gdt.h"
#include "include/idt.h"
#include "include/keyboard.h"
#include "include/mouse.h"
#include "memory_map.h"
#include "include/pit.h"
#include "include/shell.h"
#include "pmm.h"
#include "multiboot2.h"

#include <stdint.h>

#define MULTIBOOT2_MAGIC 0x36D76289

#define PIC1_DATA 0x21
#define PIC2_DATA 0xA1

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* ------------------------------------------------------------ */

static void print_ok(const char *msg)
{
    vga_puts_color("[ OK ] ", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(msg);
    vga_putchar('\n');
}

/* ------------------------------------------------------------ */

static void kbd_isr(interrupt_frame_t *frame)
{
    (void)frame;
    keyboard_handle_irq();
}

static void mouse_isr(interrupt_frame_t *frame)
{
    (void)frame;
    pit_handler();
    mouse_handle_irq();
}

static void pit_isr(interrupt_frame_t *frame)
{
    (void)frame;
    pit_handler();
}





/* ------------------------------------------------------------ */

void kernel_main(uint32_t magic, uint64_t addr)
{
   /* (void)addr;*/

    vga_init();

    vga_puts_color(
        "====================================================\n",
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_BLACK
    );

    vga_puts_color(
        " AIOS Autonomous Intelligent Operating System\n",
        VGA_COLOR_WHITE,
        VGA_COLOR_BLACK
    );

    vga_puts_color(
        " Phase 1 : Foundation\n",
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_BLACK
    );

    vga_puts_color(
        "====================================================\n\n",
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_BLACK
    );

    /* -------------------------------------------------------- */
    /* Multiboot */

    if (1) {
        print_ok("Multiboot2 verified");
	vga_puts("CALLING MEMORY MAP...\n");
	print_memory_map(addr);
	vga_puts("\nMEMORY MAP DONE\n\n");
	for (;;)
{
    __asm__ volatile ("hlt");
}
    } else {
        vga_puts_color(
            "[WARN] Not booted via Multiboot2\n",
            VGA_COLOR_BROWN,
            VGA_COLOR_BLACK
        );
    }

    /* -------------------------------------------------------- */
    /* GDT */

    print_ok("before gdt");

    gdt_init();

    print_ok("after gdt");

    /* -------------------------------------------------------- */
    /* IDT */

    print_ok("before idt");

    idt_init();

    print_ok("after idt");
    pmm_debug_set_free(131072);
    idt_register_handler(0x20, pit_isr);
    pit_init(100);

    /* -------------------------------------------------------- */
    /* KEYBOARD */

    print_ok("before keyboard");

    idt_register_handler(0x21, kbd_isr);

    keyboard_init();

    print_ok("after keyboard");

    /* -------------------------------------------------------- */
    /* MOUSE */

    idt_register_handler(0x2C, mouse_isr);

    mouse_init();

    print_ok("mouse initialized");

    /* -------------------------------------------------------- */
    /* PIC UNMASK */

    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    mask1 &= ~(1 << 0);
    mask1 &= ~(1 << 1);
    mask1 &= ~(1 << 2);
    mask2 &= ~(1 << 4);

    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);

    print_ok("PIC IRQs unmasked");

    /* -------------------------------------------------------- */
    /* ENABLE INTERRUPTS */

    print_ok("before sti");

    __asm__ volatile ("sti");

    print_ok("interrupts enabled");

    /* -------------------------------------------------------- */

    vga_putchar('\n');

    vga_puts_color(
        "Keyboard + Mouse subsystem active\n",
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_BLACK
    );
    shell_init();

    vga_puts_color(
        "Type something...\n",
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_BLACK
    );

    /* -------------------------------------------------------- */

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
