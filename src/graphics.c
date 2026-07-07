// graphics.c
#include "graphics.h"
#include "gpu.h"
#include "string.h"

// 프론트버퍼 (물리 프레임버퍼 매핑 주소; GOP/VBE에서 가져온다고 가정)
static uint32_t *framebuffer = NULL;
static int fb_width = 0, fb_height = 0, fb_pitch = 0;

void graphics_init(uint32_t *fb, int width, int height, int pitch) {
    framebuffer = fb;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch; // 바이트 단위; 픽셀 단위면 sizeof(uint32_t) 곱해서 사용
}

// ==========================================
// 기존 호환성 함수 (프론트버퍼 직접 조작)
// ==========================================
void draw_pixel(int x, int y, uint32_t color) {
    if (!framebuffer) return;
    if (x >= 0 && x < fb_width && y >= 0 && y < fb_height) {
        // pitch 고려: 실제 메모리 상의 오프셋 계산 (단순화를 위해 pitch 무시 버전과 pitch 고려 버전 분기)
        // framebuffer[y * fb_width + x] = color; 
        uint8_t *ptr = (uint8_t *)framebuffer + (y * fb_pitch) + (x * sizeof(uint32_t));
        *((uint32_t *)ptr) = color;
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    if (!framebuffer) return;
    graphics_fill_rect_on_buffer(framebuffer, fb_width, fb_height, x, y, width, height, color);
}

// 간단한 8x8 비트맵 폰트 스텁 (전체 ASCII 채워넣으면 완벽한 폰트가 됨)
static const uint8_t font8x8[128][8] = {
    ['A'] = { 0b01111100, 0b10001010, 0b10001010, 0b10001010, 0b11111110, 0b10001010, 0b10001010, 0b00000000 },
    ['B'] = { 0b11111100, 0b10000010, 0b10000010, 0b11111100, 0b10000010, 0b10000010, 0b11111100, 0b00000000 },
    ['C'] = { 0b01111100, 0b10000010, 0b10000001, 0b10000001, 0b10000001, 0b10000010, 0b01111100, 0b00000000 },
    ['D'] = { 0b11111100, 0b10000010, 0b10000001, 0b10000001, 0b10000001, 0b10000010, 0b11111100, 0b00000000 },
    ['E'] = { 0b11111110, 0b10000001, 0b10000001, 0b11111110, 0b10000001, 0b10000001, 0b11111110, 0b00000000 },
    ['F'] = { 0b11111110, 0b10000001, 0b10000001, 0b11111110, 0b10000000, 0b10000000, 0b10000000, 0b00000000 },
    [' '] = { 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000 },
    ['0'] = { 0b01111100, 0b10001010, 0b10010010, 0b10010010, 0b10010010, 0b10001010, 0b01111100, 0b00000000 },
    ['1'] = { 0b00100010, 0b01100010, 0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b01111110, 0b00000000 },
    // 나머지 문자는 기본적으로 빈칸으로 처리됨
};

static void draw_char_on_buffer(uint32_t *buf, int buf_w, int buf_h, char c, int x, int y, uint32_t fg) {
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

void draw_char(char c, int x, int y, uint32_t color) {
    if (!framebuffer) return;
    draw_char_on_buffer(framebuffer, fb_width, fb_height, c, x, y, color);
}

void draw_string(const char *str, int x, int y, uint32_t color) {
    if (!framebuffer) return;
    graphics_draw_string_on_buffer(framebuffer, fb_width, fb_height, str, x, y, color);
}

void draw_number(uint64_t num, int x, int y, uint32_t color) {
    char buf[21];
    int i = 0;
    if (num == 0) {
        draw_char('0', x, y, color);
        return;
    }
    while (num > 0 && i < 20) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    buf[i] = '\0';
    // 역순 출력
    for (int j = 0; j < i; j++) {
        draw_char(buf[i - 1 - j], x + (j * 8), y, color);
    }
}

void swap_buffers(void) {
    graphics_swap_buffers();
}

// ==========================================
// 새로운 더블버퍼링 / 버퍼 조작용 함수 구현
// ==========================================
void graphics_clear_backbuffer(uint32_t color, uint32_t *buf, int bw, int bh) {
    if (!buf) return;
    for (int i = 0; i < bw * bh; i++) {
        buf[i] = color;
    }
}

void graphics_fill_rect_on_buffer(uint32_t *buf, int buf_w, int buf_h, int x, int y, int w, int h, uint32_t color) {
    if (!buf) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > buf_w) w = buf_w - x;
    if (y + h > buf_h) h = buf_h - y;
    if (w <= 0 || h <= 0) return;

    // 최적화: 한 줄씩 memcpy로 빠르게 채우기
    uint32_t row_buf[4096]; // 한 줄 최대 크기 가정
    for (int c = 0; c < w; c++) row_buf[c] = color;
    
    for (int row = 0; row < h; row++) {
        int base = (y + row) * buf_w + x;
        memcpy(&buf[base], row_buf, w * sizeof(uint32_t));
    }
}

void graphics_draw_string_on_buffer(uint32_t *buf, int buf_w, int buf_h, const char *s, int x, int y, uint32_t fg) {
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

int graphics_measure_string(const char *s) {
    if (!s) return 0;
    int len = 0;
    while (*s) { len++; s++; }
    return len * 8;
}

void graphics_copy_buffer_to_framebuffer(const uint32_t *src, int src_w, int src_h) {
    if (!framebuffer || !src) return;
    int copy_w = (src_w < fb_width) ? src_w : fb_width;
    int copy_h = (src_h < fb_height) ? src_h : fb_height;
    for (int y = 0; y < copy_h; y++) {
        // pitch가 다를 경우를 대비해 바이트 단위로 정확하게 계산
        uint8_t *dst_ptr = (uint8_t *)framebuffer + (y * fb_pitch);
        const uint8_t *src_ptr = (const uint8_t *)src + (y * src_w * sizeof(uint32_t));
        memcpy(dst_ptr, src_ptr, copy_w * sizeof(uint32_t));
    }
}

void graphics_swap_buffers(void) {
    // compositor가 백버퍼→프론트버퍼 복사를 직접 수행하므로 여기서는 빈 상태
}
