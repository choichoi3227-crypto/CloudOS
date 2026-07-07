// graphics.c
#include "graphics.h"
#include "gpu.h"
#include <string.h>

// 프론트버퍼 (물리 프레임버퍼 매핑 주소; GOP/VBE에서 가져온다고 가정)
static uint32_t *framebuffer = NULL;
static int fb_width = 0, fb_height = 0, fb_pitch = 0;

void graphics_init(uint32_t *fb, int width, int height, int pitch) {
    framebuffer = fb;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch; // 바이트 단위; 픽셀 단위면 sizeof(uint32_t) 곱해서 사용
}

// 백버퍼 전체 초기화
void graphics_clear_backbuffer(uint32_t color, uint32_t *buf, int bw, int bh) {
    if (!buf) return;
    for (int i = 0; i < bw * bh; i++) {
        buf[i] = color;
    }
}

// 지정 버퍼에 사각형 채우기
void graphics_fill_rect_on_buffer(uint32_t *buf, int buf_w, int buf_h,
                                  int x, int y, int w, int h, uint32_t color) {
    if (!buf) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > buf_w) w = buf_w - x;
    if (y + h > buf_h) h = buf_h - y;
    if (w <= 0 || h <= 0) return;
    for (int row = 0; row < h; row++) {
        int base = (y + row) * buf_w + x;
        for (int col = 0; col < w; col++) {
            buf[base + col] = color;
        }
    }
}

// 프론트버퍼에 직접 사각형 채우기 (거의 안 쓰게 됨; 백버퍼 경로 사용)
void graphics_fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (!framebuffer) return;
    graphics_fill_rect_on_buffer(framebuffer, fb_width, fb_height, x, y, w, h, color);
}

// 간단한 8x8 비트맵 폰트를 위한 스텁 (실제 폰트 테이블 필요)
static const uint8_t font8x8[128][8] = {
    // TODO: 실제 8x8 폰트 데이터를 채워 넣기.
    // 여기서는 'A'만 예시로 넣음.
    ['A'] = {
        0b01111100,
        0b10001010,
        0b10001010,
        0b10001010,
        0b11111110,
        0b10001010,
        0b10001010,
        0b00000000,
    },
};

static void draw_char_on_buffer(uint32_t *buf, int buf_w, int buf_h,
                                char c, int x, int y, uint32_t fg) {
    if (c < 0 || c >= 128) return;
    uint8_t *glyph = (uint8_t *)font8x8[c];
    for (int row = 0; row < 8; row++) {
        uint8_t mask = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (mask & (0x80 >> col)) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < buf_w && py >= 0 && py < buf_h) {
                    buf[py * buf_w + px] = fg;
                }
            }
        }
    }
}

// 백버퍼에 문자열 그리기
void graphics_draw_string_on_buffer(uint32_t *buf, int buf_w, int buf_h,
                                    const char *s, int x, int y, uint32_t fg) {
    if (!buf || !s) return;
    int cx = x, cy = y;
    while (*s) {
        if (*s == '\n') {
            cx = x;
            cy += 10; // 간단한 줄바꿈 높이
        } else {
            draw_char_on_buffer(buf, buf_w, buf_h, *s, cx, cy, fg);
            cx += 8;
        }
        s++;
    }
}

// 프론트버퍼에 직접 문자열 그리기
void graphics_draw_string(const char *s, int x, int y, uint32_t fg) {
    if (!framebuffer) return;
    graphics_draw_string_on_buffer(framebuffer, fb_width, fb_height, s, x, y, fg);
}

// 문자열 너비 측정 (폰트가 8px 고정이므로 길이*8)
int graphics_measure_string(const char *s) {
    if (!s) return 0;
    int len = 0;
    while (*s) { len++; s++; }
    return len * 8;
}

// 백버퍼를 프론트버퍼로 복사 (소프트웨어 폴백)
void graphics_copy_buffer_to_framebuffer(const uint32_t *src, int src_w, int src_h) {
    if (!framebuffer || !src) return;
    // 단순화: 프론트버퍼 크기와 src_w/src_h가 동일하다고 가정
    int copy_w = (src_w < fb_width) ? src_w : fb_width;
    int copy_h = (src_h < fb_height) ? src_h : fb_height;
    for (int y = 0; y < copy_h; y++) {
        int src_off = y * src_w;
        int dst_off = y * fb_width; // pitch를 고려해야 하지만, 여기서는 단순화
        memcpy(&framebuffer[dst_off], &src[src_off], copy_w * sizeof(uint32_t));
    }
}

// 더블버퍼링 스왑 (필요하면 GPU 블릿 경로 사용)
void graphics_swap_buffers(void) {
    // compositor가 이미 백버퍼→프론트버퍼 복사/블릿을 수행하므로,
    // 여기서는 추가 동작이 필요하지 않음 (경로 분리용).
}
