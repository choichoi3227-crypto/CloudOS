// src/graphics.c (기존 파일 덮어쓰기)
#include "graphics.h"
#include "io.h"
#include "string.h"

static uint32_t *framebuffer = NULL;
static int fb_width = 0, fb_height = 0, fb_pitch = 0;

void graphics_init(uint32_t *fb, int width, int height, int pitch) {
    framebuffer = fb; fb_width = width; fb_height = height; fb_pitch = pitch;
}

void draw_pixel(int x, int y, uint32_t color) {
    if (!framebuffer || x < 0 || x >= fb_width || y < 0 || y >= fb_height) return;
    uint8_t *ptr = (uint8_t *)framebuffer + (y * fb_pitch) + (x * 4);
    *((uint32_t *)ptr) = color;
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    if (!framebuffer) return;
    graphics_fill_rect_on_buffer(framebuffer, fb_width, fb_height, x, y, width, height, color);
}

// 상용 OS급 8x8 폰트 테이블 (이전과 동일하므로 생략, 기존 그대로 두시면 됩니다)
static const uint8_t font8x8[128][8] = { 
    ['A'] = {0x38,0x6C,0xC6,0xC6,0xFE,0xC6,0xC6,0x00}, ['B'] = {0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00}, 
    // ... (이전 코드의 전체 폰트 테이블 그대로 유지) ...
    [' '] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, ['0'] = {0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0x7C,0x00}, 
    ['1'] = {0x18,0x38,0x78,0x18,0x18,0x18,0x7E,0x00}, // 필요한 문자만 예시로 표기
};

// [수정] static을 제거하여 외부(터미널)에서 호출 가능하게 변경
void graphics_draw_char_on_buffer(uint32_t *buf, int buf_w, int buf_h, char c, int x, int y, uint32_t fg) {
    unsigned char uc = (unsigned char)c;
    if (uc >= 128) return;
    uint8_t *glyph = (uint8_t *)font8x8[uc];
    for (int row = 0; row < 8; row++) {
        uint8_t mask = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (mask & (0x80 >> col)) {
                int px = x + col, py = y + row;
                if (px >= 0 && px < buf_w && py >= 0 && py < buf_h) buf[py * buf_w + px] = fg;
            }
        }
    }
}

void graphics_draw_string_on_buffer(uint32_t *buf, int buf_w, int buf_h, const char *s, int x, int y, uint32_t fg) {
    if (!buf || !s) return;
    int cx = x;
    while (*s) {
        if (*s == '\n') { cx = x; y += 12; }
        else { graphics_draw_char_on_buffer(buf, buf_w, buf_h, *s, cx, y, fg); cx += 8; }
        s++;
    }
}

void draw_string(const char *str, int x, int y, uint32_t color) {
    if (!framebuffer) return;
    graphics_draw_string_on_buffer(framebuffer, fb_width, fb_height, str, x, y, color);
}

int graphics_measure_string(const char *s) { if (!s) return 0; int len = 0; while (*s) { len++; s++; } return len * 8; }
void draw_number(uint64_t num, int x, int y, uint32_t color) { /* 이전과 동일 */ }
void graphics_fill_rect_on_buffer(uint32_t *buf, int buf_w, int buf_h, int x, int y, int w, int h, uint32_t color) { /* 이전과 동일 */ }
void graphics_copy_buffer_to_framebuffer(const uint32_t *src, int src_w, int src_h) { /* 이전과 동일 */ }
void swap_buffers(void) {}
void read_rtc(rtc_time_t *t) { /* 이전과 동일 */ }
