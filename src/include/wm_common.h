// wm_common.h
#ifndef WM_COMMON_H
#define WM_COMMON_H

#include "types.h"

#define SCREEN_W 1024
#define SCREEN_H 768

// 상용 OS급 색상 팔레트 (Windows 11 / macOS Ventura 스타일)
#define COLOR_TRANSPARENT  0x00000000u
#define COLOR_BLACK        0xFF000000u
#define COLOR_WHITE        0xFFFFFFFFu
#define COLOR_DARK_BG      0xFF202020u
#define COLOR_LIGHT_BG     0xFFF3F3F3u
#define COLOR_TITLEBAR_ACT 0xFF0078D4u // 활성 창 타이틀바 (액센트 블루)
#define COLOR_TITLEBAR_INC 0xFF2D2D2Du // 비활성 창 타이틀바
#define COLOR_CLIENT_DARK  0xFF1E1E1Eu
#define COLOR_CLIENT_LIGHT 0xFFFFFFFFu
#define COLOR_SHADOW       0x40000000u // 25% 투명도 그림자
#define COLOR_TASKBAR_BG   0xFF1F1F1Fu
#define COLOR_TASKBAR_BTN  0xFF333333u
#define COLOR_BTN_CLOSE    0xFFC42B1Cu
#define COLOR_BTN_MAX      0xFF00000000u
#define COLOR_TEXT_WHITE   0xFFFFFFFFu
#define COLOR_TEXT_GRAY    0xFF999999u

typedef enum {
    WIN_STATE_NORMAL = 0,
    WIN_STATE_MINIMIZED,
    WIN_STATE_MAXIMIZED,
    WIN_STATE_FULLSCREEN
} window_state_t;

typedef struct {
    int id;
    int x, y, w, h;
    window_state_t state;
    int desktop_id;
    int is_dark_mode;
    char title[128];
    uint32_t *pixels;
    int pixels_dirty;
    int is_focused; // 상용 OS 필수: 포커스 상태
} wm_window_t;

typedef enum {
    WM_EVENT_NONE = 0,
    WM_EVENT_MOUSE_MOVE,
    WM_EVENT_MOUSE_DOWN,
    WM_EVENT_MOUSE_UP,
    WM_EVENT_KEY_DOWN,
    WM_EVENT_KEY_UP,
    WM_EVENT_WINDOW_CLOSE,
    WM_EVENT_WINDOW_MAXIMIZE,
    WM_EVENT_WINDOW_MINIMIZE,
    WM_EVENT_DESKTOP_SWITCH
} wm_event_type_t;

typedef struct {
    wm_event_type_t type;
    int x, y;
    int dx, dy;
    int button;
    int key;
    int window_id;
} wm_event_t;

#define WM_MAX_WINDOWS 64
#define WM_MAX_DESKTOPS 4
#define TASKBAR_H 40
#define TITLEBAR_H 32
#define BORDER_W 1
#define MIN_WIN_W 200
#define MIN_WIN_H 150

// 상용 OS 필수: 원형 이벤트 큐 (인터럽트에서 안전하게 push)
#define WM_EVENT_QUEUE_SIZE 1024
typedef struct {
    wm_event_t events[WM_EVENT_QUEUE_SIZE];
    int head;
    int tail;
} wm_event_queue_t;

void wm_event_queue_init(wm_event_queue_t *q);
int wm_event_queue_push(wm_event_queue_t *q, wm_event_t *ev);
int wm_event_queue_pop(wm_event_queue_t *q, wm_event_t *out);

#endif // WM_COMMON_H
