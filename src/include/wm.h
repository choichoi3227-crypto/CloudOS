// wm.h
#ifndef WM_H
#define WM_H

#include "types.h"
#include "wm_common.h" // wm_event_t 등 공용 타입 사용

// WM 초기화 및 메인 루프
void wm_init(void);
void wm_run(void);

// 앱에서 창을 제어하기 위한 API
int wm_create_window(const char *title, int x, int y, int w, int h, int desktop_id, int is_dark_mode);
int wm_close_window(int id);
int wm_update_window(int id, int x, int y, int w, int h, const char *title, int is_dark_mode);

#endif
