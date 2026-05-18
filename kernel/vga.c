/* ============================================================
 * AIOS — VGA Text Mode Driver  (80×25, colour)
 * Provides: vga_init, vga_putchar, vga_puts, vga_puts_color,
 *           vga_puthex, vga_putdec, vga_backspace,
 *           vga_set_cursor_pos, vga_clear
 * ============================================================ */

#include "include/vga.h"
#include <stdint.h>
#include <stddef.h>

/* ── Hardware constants ──────────────────────────────────── */
#define VGA_WIDTH    80
#define VGA_HEIGHT   25
#define VGA_MEMORY   ((volatile uint16_t *)0xB8000)

#define VGA_REG_CTRL  0x3D4
#define VGA_REG_DATA  0x3D5

/* ── State ───────────────────────────────────────────────── */
static size_t    col  = 0;
static size_t    row  = 0;
static uint8_t   color;
static volatile uint16_t *vga_buf;

/* ── I/O helpers ─────────────────────────────────────────── */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t v; __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port)); return v;
}

/* ── Helpers ─────────────────────────────────────────────── */
static inline uint16_t vga_entry(unsigned char c, uint8_t col_attr) {
    return (uint16_t)c | ((uint16_t)col_attr << 8);
}
static inline uint8_t vga_color(vga_color_t fg, vga_color_t bg) {
    return (uint8_t)fg | ((uint8_t)bg << 4);
}

/* ── Hardware cursor ─────────────────────────────────────── */
static void hw_cursor_update(void) {
    uint16_t pos = (uint16_t)(row * VGA_WIDTH + col);
    outb(VGA_REG_CTRL, 0x0F); outb(VGA_REG_DATA, (uint8_t)(pos & 0xFF));
    outb(VGA_REG_CTRL, 0x0E); outb(VGA_REG_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

/* Enable blinking hardware text cursor */
static void hw_cursor_enable(void) {
    outb(VGA_REG_CTRL, 0x0A);
    outb(VGA_REG_DATA, (inb(VGA_REG_DATA) & 0xC0) | 0x0D);   /* scanline 13 */
    outb(VGA_REG_CTRL, 0x0B);
    outb(VGA_REG_DATA, (inb(VGA_REG_DATA) & 0xE0) | 0x0F);   /* scanline 15 */
}

/* ── Scroll one line ─────────────────────────────────────── */
static void scroll(void) {
    for (size_t r = 1; r < VGA_HEIGHT; r++)
        for (size_t c = 0; c < VGA_WIDTH; c++)
            vga_buf[(r-1)*VGA_WIDTH + c] = vga_buf[r*VGA_WIDTH + c];
    for (size_t c = 0; c < VGA_WIDTH; c++)
        vga_buf[(VGA_HEIGHT-1)*VGA_WIDTH + c] = vga_entry(' ', color);
    row = VGA_HEIGHT - 1;
}

/* ── vga_init ────────────────────────────────────────────── */
void vga_init(void) {
    vga_buf = VGA_MEMORY;
    color   = vga_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    col = row = 0;

    /* Clear screen */
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buf[i] = vga_entry(' ', color);

    hw_cursor_enable();
    hw_cursor_update();
}

/* ── vga_clear ───────────────────────────────────────────── */
void vga_clear(void) {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga_buf[i] = vga_entry(' ', color);
    col = row = 0;
    hw_cursor_update();
}

/* ── vga_set_color ───────────────────────────────────────── */
void vga_set_color(vga_color_t fg, vga_color_t bg) {
    color = vga_color(fg, bg);
}

/* ── vga_set_cursor_pos ──────────────────────────────────── */
void vga_set_cursor_pos(size_t c, size_t r) {
    col = c; row = r;
    hw_cursor_update();
}

/* ── vga_putchar ─────────────────────────────────────────── */
void vga_putchar(char c) {
    if (c == '\n') {
        col = 0;
        if (++row >= VGA_HEIGHT) scroll();
    } else if (c == '\r') {
        col = 0;
    } else if (c == '\t') {
        col = (col + 8) & ~7u;
        if (col >= VGA_WIDTH) { col = 0; if (++row >= VGA_HEIGHT) scroll(); }
    } else if (c == '\b') {
        if (col > 0) { col--; }
        else if (row > 0) { row--; col = VGA_WIDTH - 1; }
        vga_buf[row * VGA_WIDTH + col] = vga_entry(' ', color);
    } else {
        vga_buf[row * VGA_WIDTH + col] = vga_entry((unsigned char)c, color);
        if (++col >= VGA_WIDTH) { col = 0; if (++row >= VGA_HEIGHT) scroll(); }
    }
    hw_cursor_update();
}

/* ── vga_backspace (public helper for keyboard echo) ─────── */
void vga_backspace(void) {
    vga_putchar('\b');
}

/* ── vga_puts ────────────────────────────────────────────── */
void vga_puts(const char *s) {
    while (*s) vga_putchar(*s++);
}

/* ── vga_puts_color ──────────────────────────────────────── */
void vga_puts_color(const char *s, vga_color_t fg, vga_color_t bg) {
    uint8_t saved = color;
    color = vga_color(fg, bg);
    while (*s) vga_putchar(*s++);
    color = saved;
}

/* ── vga_puthex ──────────────────────────────────────────── */
void vga_puthex(uint64_t val) {
    static const char hex[] = "0123456789ABCDEF";
    vga_puts("0x");
    for (int i = 60; i >= 0; i -= 4)
        vga_putchar(hex[(val >> i) & 0xF]);
}

/* ── vga_putdec ──────────────────────────────────────────── */
void vga_putdec(uint64_t val) {
    if (val == 0) { vga_putchar('0'); return; }
    char buf[20]; int i = 0;
    while (val > 0) { buf[i++] = '0' + (char)(val % 10); val /= 10; }
    while (i--) vga_putchar(buf[i]);
}


