#ifndef WM_H
#define WM_H
#include "types.h"

#define MAX_WINDOWS 5

typedef struct {
    int x, y, width, height;
    char title[32];
    int is_open;
    int is_dragging;
    int drag_offset_x, drag_offset_y;
    int close_clicked; // 추가: 창 닫기 요청 플래그
} window_t;

void wm_init(void);
int wm_create_window(int x, int y, int w, int h, const char* title);
void wm_render(void);
void wm_handle_mouse(int mx, int my, int left_btn_down);

#endif
