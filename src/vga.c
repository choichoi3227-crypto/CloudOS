#include "vga.h"

static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static int vga_row = 0;
static int vga_col = 0;
static uint8_t vga_color = 0x0F;

void vga_init(void) {
    vga_row = 0;
    vga_col = 0;
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            VGA_MEMORY[y * VGA_WIDTH + x] = ((uint16_t)vga_color << 8) | ' ';
        }
    }
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color = fg | (bg << 4);
}

void vga_print_char(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else {
        VGA_MEMORY[vga_row * VGA_WIDTH + vga_col] = ((uint16_t)vga_color << 8) | c;
        vga_col++;
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
    }
    if (vga_row >= VGA_HEIGHT) {
        vga_init(); // 간단화된 스크롤
    }
}

void vga_print(const char* str) {
    while (*str) {
        vga_print_char(*str++);
    }
}
