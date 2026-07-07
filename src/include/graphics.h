// graphics.h
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"
#include "wm_common.h" // SCREEN_W, SCREEN_H 등 공유 매크로 사용

// 하위 호환성을 위한 매크로 (기존 코드 깨짐 방지)
#define SCREEN_WIDTH SCREEN_W
#define SCREEN_HEIGHT SCREEN_H
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

// 프론트버퍼 주소, 가로/세로, 피치(바이트)를 받는 버전으로 통일
void graphics_init(uint32_t *fb, int width, int height, int pitch);

// 기존 프론트버퍼 직접 그리기 함수 (호환성 유지)
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_char(char c, int x, int y, uint32_t color);
void draw_string(const char *str, int x, int y, uint32_t color);
void draw_number(uint64_t num, int x, int y, uint32_t color);
void swap_buffers(void);

// ==========================================
// 새로운 더블버퍼링 / 버퍼 조작용 함수
// ==========================================
void graphics_clear_backbuffer(uint32_t color, uint32_t *buf, int bw, int bh);
void graphics_fill_rect_on_buffer(uint32_t *buf, int buf_w, int buf_h,
                                  int x, int y, int w, int h, uint32_t color);
void graphics_draw_string_on_buffer(uint32_t *buf, int buf_w, int buf_h,
                                    const char *s, int x, int y, uint32_t fg);
int graphics_measure_string(const char *s);
void graphics_copy_buffer_to_framebuffer(const uint32_t *src, int src_w, int src_h);
void graphics_swap_buffers(void);

#endif
