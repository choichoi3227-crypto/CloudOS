// src/apps/terminal.h (새 파일 생성)
#ifndef TERMINAL_APP_H
#define TERMINAL_APP_H

#include "../include/wm_common.h"

#define TERM_COLS 80
#define TERM_ROWS 25
#define TERM pixel(x,y) (term_buffer[(y) * TERM_COLS + (x)])

typedef struct {
    char text_buffer[TERM_ROWS][TERM_COLS];
    int cursor_x;
    int cursor_y;
    char input_buffer[256];
    int input_idx;
    int scroll_offset;
} terminal_state_t;

// 터미널 초기화 및 화면 그리기 함수
void terminal_init(wm_window_t *win);
void terminal_draw(wm_window_t *win);

// WM에서 호출할 키 입력 핸들러 (wm_common.h의 콜백 타입에 맞음)
void terminal_key_handler(int key, void *user_data);

#endif
