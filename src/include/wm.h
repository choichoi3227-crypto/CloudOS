// wm.h
#ifndef WM_H
#define WM_H

#include "types.h"
#include "wm_common.h"

void wm_init(void);
void wm_run(void);
int wm_create_window(const char *title, int x, int y, int w, int h, int desktop_id, int is_dark_mode);
int wm_close_window(int id);

// 인터럽트 핸들러에서 호출할 수 있는 퍼블릭 큐 인터페이스
void wm_push_input_event(wm_event_t *ev);

#endif
