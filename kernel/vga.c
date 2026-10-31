/* ============================================================
   vga.c - VGA text-mode terminal: cursor tracking, newline,
   backspace, and scrolling once text reaches the bottom row.
   ============================================================ */
#include <stdint.h>
#include "vga.h"

#define VGA_MEMORY   ((uint16_t*)0xB8000)
#define VGA_COLS     80
#define VGA_ROWS     25
#define VGA_COLOR    0x0F

static int cursor_row = 0;
static int cursor_col = 0;

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_init(void) {
    uint16_t *vga = VGA_MEMORY;
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga[i] = vga_entry(' ', 0x00);
    }
    cursor_row = 0;
    cursor_col = 0;
}

static void vga_scroll(void) {
    uint16_t *vga = VGA_MEMORY;
    for (int row = 1; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            vga[(row - 1) * VGA_COLS + col] = vga[row * VGA_COLS + col];
        }
    }
    for (int col = 0; col < VGA_COLS; col++) {
        vga[(VGA_ROWS - 1) * VGA_COLS + col] = vga_entry(' ', 0x00);
    }
    cursor_row = VGA_ROWS - 1;
}

void vga_putc(char c) {
    uint16_t *vga = VGA_MEMORY;

    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
    } else if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            vga[cursor_row * VGA_COLS + cursor_col] = vga_entry(' ', 0x00);
        }
    } else {
        vga[cursor_row * VGA_COLS + cursor_col] = vga_entry(c, VGA_COLOR);
        cursor_col++;
        if (cursor_col >= VGA_COLS) {
            cursor_col = 0;
            cursor_row++;
        }
    }

    if (cursor_row >= VGA_ROWS) {
        vga_scroll();
    }
}

void vga_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putc(str[i]);
    }
}
