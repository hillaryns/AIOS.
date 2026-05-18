/* ============================================================
 * AIOS — Interrupt Descriptor Table (IDT)
 * ============================================================ */

#include "include/idt.h"
#include "include/vga.h"
#include "exceptions.h"
#include <stdint.h>

/* ============================================================
 * ISR STUB TABLE
 * ============================================================ */

extern void *isr_stub_table[];

/* ============================================================
 * IDT ENTRY
 * ============================================================ */

typedef struct __attribute__((packed))
{
    uint16_t offset_low;
    uint16_t selector;

    uint8_t  ist;
    uint8_t  type_attr;

    uint16_t offset_mid;
    uint32_t offset_high;

    uint32_t zero;

} idt_entry_t;

/* ============================================================
 * IDT POINTER
 * ============================================================ */

typedef struct __attribute__((packed))
{
    uint16_t limit;
    uint64_t base;

} idt_ptr_t;

/* ============================================================
 * CONSTANTS
 * ============================================================ */

#define IDT_INTERRUPT_GATE 0x8E

/* ============================================================
 * STORAGE
 * ============================================================ */

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

static isr_handler_t isr_handlers[IDT_ENTRIES] = {0};

/* ============================================================
 * PORT I/O
 * ============================================================ */

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(val), "Nd"(port)
    );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(ret)
        : "Nd"(port)
    );

    return ret;
}

/* ============================================================
 * PIC
 * ============================================================ */

#define PIC1_CMD   0x20
#define PIC1_DATA  0x21

#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1

#define PIC_EOI    0x20

static void pic_remap(int offset1, int offset2)
{
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);

    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

/* ============================================================
 * IDT GATE SETTER
 * ============================================================ */

static void idt_set_gate(
    uint8_t vec,
    void *isr,
    uint8_t flags
)
{
    uint64_t addr = (uint64_t)isr;

    idt[vec].offset_low  = addr & 0xFFFF;
    idt[vec].selector    = 0x08;

    idt[vec].ist         = 0;
    idt[vec].type_attr   = flags;

    idt[vec].offset_mid  = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high = (addr >> 32) & 0xFFFFFFFF;

    idt[vec].zero = 0;
}

/* ============================================================
 * ISR DISPATCH
 * ============================================================ */

void isr_dispatch(interrupt_frame_t *frame)
{
    uint64_t vec = frame->int_num;

    /* CPU Exceptions */
    if (vec < 32) {

        exception_handler(vec);
    }

    /* Registered ISR handlers */
    if (isr_handlers[vec]) {

        isr_handlers[vec](frame);
    }

    /* send EOI for hardware IRQs */
    if (vec >= 32 && vec < 48) {

        if (vec >= 40) {
            outb(PIC2_CMD, PIC_EOI);
        }

        outb(PIC1_CMD, PIC_EOI);
    }
}

/* ============================================================
 * REGISTER ISR
 * ============================================================ */

void idt_register_handler(
    uint8_t vector,
    isr_handler_t handler
)
{
    isr_handlers[vector] = handler;
}

/* ============================================================
 * IDT INIT
 * ============================================================ */

void idt_init(void)
{
    vga_puts_color(
        "  [ OK ] entering idt_init\n",
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_BLACK
    );

    /* remap PIC */
    pic_remap(0x20, 0x28);

    vga_puts_color(
        "  [ OK ] pic remapped\n",
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_BLACK
    );

    vga_puts_color(
        "  [ OK ] installing ISR gates\n",
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_BLACK
    );

    /* install all ISR stubs */
    for (int i = 0; i < IDT_ENTRIES; i++) {

        idt_set_gate(
            i,
            isr_stub_table[i],
            IDT_INTERRUPT_GATE
        );

        if (i == 255) {

            vga_puts_color(
                "  [ OK ] last ISR gate installed\n",
                VGA_COLOR_LIGHT_GREEN,
                VGA_COLOR_BLACK
            );
        }
    }

    /* setup IDT pointer */
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint64_t)&idt;

    /* load IDT */
    __asm__ volatile (
        "lidt %0"
        :
        : "m"(idt_ptr)
    );

    vga_puts_color(
        "  [ OK ] lidt success\n",
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_BLACK
    );

    vga_puts_color(
        "  [ OK ] IDT loaded successfully\n",
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_BLACK
    );
}
