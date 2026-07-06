#include "graphics.h"
#include "string.h"
#include "heap.h"

static uint32_t* framebuffer;
static uint32_t* backbuffer;
static uint32_t fb_pitch;

// 간략화된 8x16 폰트 (ASCII 32~127)
// 테스트를 위해 일부 문자만 정의, 나머지는 0 처리
static uint8_t font8x16[96][16] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Space
    {0x00,0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x00}, // A
    {0x00,0x7C,0x66,0x66,0x66,0x7C,0x66,0x66,0x66,0x7C,0x00,0x00,0x00,0x00,0x00,0x00}  // B
    // 실제 OS에서는 전체 폰트 테이블이 포함됨
};

void graphics_init(uint64_t fb_addr, uint32_t pitch) {
    framebuffer = (uint32_t*)fb_addr;
    fb_pitch = pitch / 4;
    // 백버퍼 메모리 할당 (더블 버퍼링)
    backbuffer = (uint32_t*)kmalloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        backbuffer[y * fb_pitch + x] = color;
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            draw_pixel(x + j, y + i, color);
        }
    }
}

void draw_char(char c, int x, int y, uint32_t color) {
    if (c < 32 || c > 127) return;
    uint8_t* glyph = font8x16[c - 32];
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            if (glyph[i] & (1 << (7 - j))) {
                draw_pixel(x + j, y + i, color);
            }
        }
    }
}

void draw_string(const char* str, int x, int y, uint32_t color) {
    int cx = x;
    while (*str) {
        draw_char(*str, cx, y, color);
        cx += FONT_WIDTH;
        str++;
    }
}

void swap_buffers(void) {
    memcpy(framebuffer, backbuffer, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
}

void draw_number(uint64_t num, int x, int y, uint32_t color) {
    char buf[20];
    int i = 0;
    if (num == 0) {
        draw_char('0', x, y, color);
        return;
    }
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    int cx = x;
    while (i > 0) {
        draw_char(buf[--i], cx, y, color);
        cx += FONT_WIDTH;
    }
}
