#ifndef COMPOSITOR_H
#define COMPOSITOR_H
#include "types.h"

#define MAX_VIRTUAL_DESKTOPS 4
#define MAX_WINDOWS 16

typedef struct {
    int x, y, w, h;
    char title[32];
    int desktop_id; // 어떤 가상 데스크탑에 있는지
    int is_open;
    int is_maximized;
    int is_dark_mode; // 다크모드 창 여부
} compositor_window_t;

void compositor_init(void);
void compositor_render(int current_desktop);
void compositor_switch_desktop(int new_desktop);
void compositor_snap_window(int win_id, int side); // 0: Left, 1: Right, 2: Max

#endif
