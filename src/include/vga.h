#ifndef VGA_H
#define VGA_H
#include <types.h>
enum vga_color { VGA_COLOR_BLACK=0, VGA_COLOR_WHITE=15, VGA_COLOR_LIGHT_CYAN=11 };
void vga_init(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_print_char(char c);
void vga_print(const char* str);
#endif
