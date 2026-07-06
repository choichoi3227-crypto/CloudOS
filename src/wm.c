#include "wm.h"
#include "graphics.h"
#include "acpi.h"
#include "string.h"

static window_t windows[MAX_WINDOWS];
static int window_count = 0;
static int active_window = -1;
static int start_menu_open = 0;

void wm_init(void) {
    window_count = 0;
    start_menu_open = 0;
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
    // 바탕화면 아이콘 (간략화)
    draw_rect(20, 40, 40, 40, 0x4A90E2);
    draw_string("Files", 20, 85, 0xFFFFFF);

    // 창 렌더링
    for (int i = 0; i < window_count; i++) {
        window_t* win = &windows[i];
        if (win->is_open) {
            draw_rect(win->x + 3, win->y + 3, win->width, win->height, 0x111111);
            draw_rect(win->x, win->y, win->width, win->height, 0xFFFFFF);
            draw_rect(win->x, win->y, win->width, 25, 0x333333);
            draw_rect(win->x + 5, win->y + 5, 15, 15, 0xFF5F56);
            draw_string(win->title, win->x + 30, win->y + 5, 0xFFFFFF);
        }
    }

    // 시작 메뉴 렌더링
    if (start_menu_open) {
        draw_rect(0, 768 - 30 - 200, 200, 200, 0xDDDDDD);
        draw_rect(0, 768 - 30 - 200, 200, 30, 0x333333);
        draw_string("CloudOS Menu", 10, 768 - 30 - 190, 0xFFFFFF);
        
        // 종료 버튼
        draw_rect(10, 768 - 30 - 150, 180, 30, 0xFF5F56);
        draw_string("Shut Down", 50, 768 - 30 - 140, 0xFFFFFF);
    }
}

void wm_handle_mouse(int mx, int my, int left_btn_down) {
    static int prev_btn = 0;
    
    if (left_btn_down && !prev_btn) {
        // 시작 메뉴 영역 클릭 (좌측 하단)
        if (mx < 50 && my > 738) {
            start_menu_open = !start_menu_open;
            return;
        }
        
        // 시작 메뉴가 열려있을 때 종료 버튼 클릭
        if (start_menu_open) {
            if (mx >= 10 && mx <= 190 && my >= 768 - 30 - 150 && my <= 768 - 30 - 120) {
                acpi_power_off(); // 시스템 종료
            }
        }

        // 창 인터랙션
        for (int i = window_count - 1; i >= 0; i--) {
            window_t* win = &windows[i];
            if (win->is_open && mx >= win->x && mx < win->x + win->width && my >= win->y && my < win->y + win->height) {
                if (mx >= win->x + 5 && mx < win->x + 20 && my >= win->y + 5 && my < win->y + 20) {
                    win->close_clicked = 1;
                    break;
                }
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
    else if (left_btn_down && prev_btn) {
        if (active_window != -1 && windows[active_window].is_dragging) {
            windows[active_window].x = mx - windows[active_window].drag_offset_x;
            windows[active_window].y = my - windows[active_window].drag_offset_y;
        }
    } 
    else if (!left_btn_down && prev_btn) {
        if (active_window != -1) {
            windows[active_window].is_dragging = 0;
            active_window = -1;
        }
        for (int i = 0; i < window_count; i++) {
            if (windows[i].close_clicked) {
                windows[i].is_open = 0;
                windows[i].close_clicked = 0;
            }
        }
    }
    prev_btn = left_btn_down;
}
