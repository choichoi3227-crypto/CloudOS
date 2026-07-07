// compositor.h
#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "types.h"
#include "wm_common.h" // wm_window_t 사용을 위해 포함

#define MAX_VIRTUAL_DESKTOPS 4

// 창 배열을 외부(wm.c)에서 받아와서 해당 데스크톱만 합성하여 그림
void compositor_init(void);
void compositor_render_desktop(int desktop_id, wm_window_t *windows, int windows_count);
void compositor_switch_desktop(int new_desktop);
void compositor_snap_window(wm_window_t *w, int side); // 0: Left, 1: Right, 2: Max

#endif
