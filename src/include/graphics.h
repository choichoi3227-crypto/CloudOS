#ifndef GRAPHICS_H
#define GRAPHICS_H
#include "types.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

void graphics_init(uint64_t fb_addr, uint32_t pitch);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_char(char c, int x, int y, uint32_t color);
void draw_string(const char* str, int x, int y, uint32_t color);
void draw_number(uint64_t num, int x, int y, uint32_t color); // 추가
void swap_buffers(void);

#endif
