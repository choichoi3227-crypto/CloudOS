#include "wm.h"
#include "graphics.h"
#include "string.h"

static window_t windows[MAX_WINDOWS];
static int window_count = 0;
static int active_window = -1;

void wm_init(void) {
    window_count = 0;
}

int wm_create_window(int x, int y, int w, int h, const char* title) {
    if (window_count >= MAX_WINDOWS) return -1;
    window_t* win = &windows[window_count];
    win->x = x; win->y = y;
    win->width = w; win->height = h;
    strcpy(win->title, title);
    win->is_open = 1;
    win->is_dragging = 0;
    win->close_clicked = 0;
    return window_count++;
}

void wm_render(void) {
    for (int i = 0; i < window_count; i++) {
        window_t* win = &windows[i];
        if (win->is_open) {
            // 그림자
            draw_rect(win->x + 3, win->y + 3, win->width, win->height, 0x111111);
            // 창 배경
            draw_rect(win->x, win->y, win->width, win->height, 0xFFFFFF);
            // 타이틀 바
            draw_rect(win->x, win->y, win->width, 25, 0x333333);
            // 닫기 버튼 (X)
            draw_rect(win->x + 5, win->y + 5, 15, 15, 0xFF5F56);
            // 타이틀 텍스트
            draw_string(win->title, win->x + 30, win->y + 5, 0xFFFFFF);
        }
    }
}

void wm_handle_mouse(int mx, int my, int left_btn_down) {
    static int prev_btn = 0;
    
    // 클릭 (드래그 시작 또는 닫기 버튼)
    if (left_btn_down && !prev_btn) {
        for (int i = window_count - 1; i >= 0; i--) {
            window_t* win = &windows[i];
            if (win->is_open && mx >= win->x && mx < win->x + win->width && my >= win->y && my < win->y + win->height) {
                
                // 닫기 버튼(X) 영역 확인
                if (mx >= win->x + 5 && mx < win->x + 20 && my >= win->y + 5 && my < win->y + 20) {
                    win->close_clicked = 1;
                    break;
                }
                
                // 타이틀 바 영역 확인 (드래그 시작)
                if (my < win->y + 25) {
                    win->is_dragging = 1;
                    win->drag_offset_x = mx - win->x;
                    win->drag_offset_y = my - win->y;
                    active_window = i;
                }
                break;
            }
        }
    } 
    // 드래그 중
    else if (left_btn_down && prev_btn) {
        if (active_window != -1 && windows[active_window].is_dragging) {
            windows[active_window].x = mx - windows[active_window].drag_offset_x;
            windows[active_window].y = my - windows[active_window].drag_offset_y;
        }
    } 
    // 클릭 해제 (드래그 종료 또는 닫기 실행)
    else if (!left_btn_down && prev_btn) {
        if (active_window != -1) {
            windows[active_window].is_dragging = 0;
            active_window = -1;
        }
        
        // 닫기 요청 처리
        for (int i = 0; i < window_count; i++) {
            if (windows[i].close_clicked) {
                windows[i].is_open = 0; // 창 닫기
                windows[i].close_clicked = 0;
            }
        }
    }
    
    prev_btn = left_btn_down;
}
