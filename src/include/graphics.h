// graphics.h
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"
#include "wm_common.h"

#define SCREEN_WIDTH SCREEN_W
#define SCREEN_HEIGHT SCREEN_H
#define FONT_WIDTH 8
#define FONT_HEIGHT 16

void graphics_init(uint32_t *fb, int width, int height, int pitch);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_string(const char *str, int x, int y, uint32_t color);
void draw_number(uint64_t num, int x, int y, uint32_t color);
void swap_buffers(void);

// 상용 OS 그래픽 파이프라인
void graphics_fill_rect_on_buffer(uint32_t *buf, int buf_w, int buf_h, int x, int y, int w, int h, uint32_t color);
void graphics_draw_string_on_buffer(uint32_t *buf, int buf_w, int buf_h, const char *s, int x, int y, uint32_t fg);
void graphics_copy_buffer_to_framebuffer(const uint32_t *src, int src_w, int src_h);
int graphics_measure_string(const char *s);

// 하드웨어 시계 (작업표시줄 시계용)
typedef struct { int year, month, day, hour, minute, second; } rtc_time_t;
void read_rtc(rtc_time_t *t);

#endif
