// compositor.h
#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "types.h"
#include "wm_common.h"

#define MAX_VIRTUAL_DESKTOPS 4

void compositor_init(void);
void compositor_render_desktop(int desktop_id, wm_window_t *windows, int windows_count);
void compositor_switch_desktop(int new_desktop);
void compositor_snap_window(wm_window_t *w, int side);

#endif
