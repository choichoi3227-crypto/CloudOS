// src/apps/terminal.c (새 파일 생성)
#include "apps/terminal.h"
#include "../include/graphics.h"
#include "../fs/vfs.h"
#include "../mm/heap.h"
#include "../include/string.h"

void terminal_init(wm_window_t *win) {
    terminal_state_t *term = (terminal_state_t *)kmalloc(sizeof(terminal_state_t));
    memset(term, 0, sizeof(terminal_state_t));
    
    // 시작 메시지 출력
    strcpy(term->text_buffer[0], "CloudOS Terminal v0.1");
    strcpy(term->text_buffer[1], "Type 'help' for commands.");
    strcpy(term->text_buffer[2], "------------------------");
    
    term->cursor_y = 3; // 커서를 입력 라인으로 이동
    
    // WM 콜백 등록 (이 창이 포커스 되고 키를 누면 이 함수가 호출됨)
    win->on_key_down = terminal_key_handler;
    win->user_data = term;
    
    // 창 픽셀 버퍼 할당 (터미널 창 크기 기준)
    win->pixels = (uint32_t *)kmalloc(win->w * win->h * sizeof(uint32_t));
    memset(win->pixels, 0, win->w * win->h * sizeof(uint32_t));
    win->pixels_dirty = 1;
}

void terminal_print(terminal_state_t *term, const char *str) {
    while (*str) {
        if (*str == '\n') {
            term->cursor_x = 0;
            term->cursor_y++;
            if (term->cursor_y >= TERM_ROWS) {
                // 스크롤: 위로 한 줄 올리고 맨 아래줄 비우기
                for (int y = 0; y < TERM_ROWS - 1; y++) {
                    memcpy(term->text_buffer[y], term->text_buffer[y + 1], TERM_COLS);
                }
                memset(term->text_buffer[TERM_ROWS - 1], ' ', TERM_COLS);
                term->cursor_y = TERM_ROWS - 1;
            }
        } else {
            term->text_buffer[term->cursor_y][term->cursor_x] = *str;
            term->cursor_x++;
            if (term->cursor_x >= TERM_COLS) {
                term->cursor_x = 0;
                term->cursor_y++;
                if (term->cursor_y >= TERM_ROWS) {
                    for (int y = 0; y < TERM_ROWS - 1; y++) memcpy(term->text_buffer[y], term->text_buffer[y + 1], TERM_COLS);
                    memset(term->text_buffer[TERM_ROWS - 1], ' ', TERM_COLS);
                    term->cursor_y = TERM_ROWS - 1;
                }
            }
        }
        str++;
    }
}

static void process_command(terminal_state_t *term, const char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        terminal_print(term, "Commands: help, clear, ls, echo [text], uname\n");
    } else if (strcmp(cmd, "clear") == 0) {
        memset(term->text_buffer, ' ', sizeof(term->text_buffer));
        term->cursor_x = 0;
        term->cursor_y = 0;
    } else if (strcmp(cmd, "ls") == 0) {
        terminal_print(term, "drwxr-xr-x  .\n");
        terminal_print(term, "drwxr-xr-x  bin\n");
        terminal_print(term, "drwxr-xr-x  sys\n");
        terminal_print(term, "drwxr-xr-x  apps\n");
        terminal_print(term, "drwxr-xr-x  fs\n");
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        terminal_print(term, cmd + 5);
        terminal_print(term, "\n");
    } else if (strcmp(cmd, "uname") == 0) {
        terminal_print(term, "CloudOS 1.0 (i686)\n");
    } else {
        terminal_print(term, "Command not found: ");
        terminal_print(term, cmd);
        terminal_print(term, "\n");
    }
}

void terminal_draw(wm_window_t *win) {
    terminal_state_t *term = (terminal_state_t *)win->user_data;
    if (!term || !win->pixels) return;
    
    int buf_w = win->w;
    int buf_h = win->h - TITLEBAR_H;
    
    // 배경 지우기
    graphics_fill_rect_on_buffer(win->pixels, buf_w, buf_h, 0, 0, buf_w, buf_h, COLOR_TERMINAL_BG);
    
    // 텍스트 버퍼 렌더링
    for (int y = 0; y < TERM_ROWS; y++) {
        for (int x = 0; x < TERM_COLS; x++) {
            char c = term->text_buffer[y][x];
            if (c != ' ') {
                // 8x8 폰트를 사용하여 글자 그리기
                graphics_draw_char_on_buffer(win->pixels, buf_w, buf_h, c, x * 8, y * 16, COLOR_TERMINAL_FG);
            }
        }
    }
    
    // 커서 깜빡깜 (현재 위치에 역삼각형 블록)
    if (term->cursor_y < TERM_ROWS && term->cursor_x < TERM_COLS) {
        graphics_fill_rect_on_buffer(win->pixels, buf_w, buf_h, 
                                    term->cursor_x * 8, term->cursor_y * 16, 8, 16, COLOR_TERMINAL_FG);
    }
    
    win->pixels_dirty = 1;
}

// WM에서 키가 눌릴 때마다 호출됨
void terminal_key_handler(int key, void *user_data) {
    terminal_state_t *term = (terminal_state_t *)user_data;
    if (!term) return;
    
    if (key == '\n') {
        term->input_buffer[term->input_idx] = '\0';
        process_command(term, term->input_buffer);
        terminal_print(term, "> ");
        term->input_idx = 0;
    } else if (key == 8) { // Backspace
        if (term->input_idx > 0) {
            term->input_idx--;
            term->input_buffer[term->input_idx] = '\0';
        }
    } else if (term->input_idx < 255) {
        term->input_buffer[term->input_idx++] = (char)key;
        // 입력 중인 글자를 실시간으로 화면에 출력 (에코 효과)
        if (term->cursor_x < TERM_COLS) {
            graphics_draw_char_on_buffer(
                ((wm_window_t*)( (uint64_t)user_data - offsetof(wm_window_t, user_data) ), // hacky way to get win ptr if needed, but we pass win as user_data
                // 정확한 방식: wm.c에서 wm_window_t*를 user_data로 넘기도록 수정했음
                ((wm_window_t*)((uint8_t*)user_data - offsetof(wm_window_t, user_data))->w,
                ((wm_window_t*)((uint8_t*)user_data - offsetof(wm_window_t, user_data))->h,
                (char)key, term->cursor_x * 8, term->cursor_y * 16, COLOR_TERMINAL_FG
            );
            term->cursor_x++;
        }
    }
}
