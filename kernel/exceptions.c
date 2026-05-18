#include "exceptions.h"
#include "vga.h"

const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Segment Fault",
    "General Protection Fault",
    "Page Fault"
};

void exception_handler(int int_no)
{
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);

    vga_puts("\n\nEXCEPTION: ");

    if (int_no < 15)
        vga_puts(exception_messages[int_no]);
    else
        vga_puts("Unknown Exception");

    vga_puts("\nSystem Halted.");

    while (1) {
        __asm__ volatile("cli; hlt");
    }
}
