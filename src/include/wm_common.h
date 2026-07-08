// src/include/wm_common.h (기존 파일 덮어쓰기)
#ifndef WM_COMMON_H
#define WM_COMMON_H

#include "types.h"

#define SCREEN_W 1024
#define SCREEN_H 768

// 색상 정의 (이전과 동일)
#define COLOR_TRANSPARENT  0x00000000u
#define COLOR_BLACK        0xFF000000u
#define COLOR_WHITE        0xFFFFFFFFu
#define COLOR_DARK_BG      0xFF202020u
#define COLOR_LIGHT_BG     0xFFF3F3F3u
#define COLOR_TITLEBAR_ACT 0xFF0078D4u
#define COLOR_TITLEBAR_INC 0xFF2D2D2Du
#define COLOR_CLIENT_DARK  0xFF1E1E1Eu
#define COLOR_CLIENT_LIGHT 0xFFFFFFFFu
#define COLOR_TASKBAR_BG   0xFF1F1F1Fu
#define COLOR_TASKBAR_BTN  0xFF333333u
#define COLOR_BTN_CLOSE    0xFFC42B1Cu
#define COLOR_TEXT_WHITE   0xFFFFFFFFu
#define COLOR_TEXT_GRAY    0xFF999999u
#define COLOR_TERMINAL_BG  0xFF0C0C0Cu // 터미널 배경색 (고전대 스타일)
#define COLOR_TERMINAL_FG  0xFF00FF00u // 터미널 글자색 (네온색)

typedef enum {
    WIN_STATE_NORMAL = 0,
    WIN_STATE_MINIMIZED,
    WIN_STATE_MAXIMIZED,
    WIN_STATE_FULLSCREEN
} window_state_t;

// [핵심 추가] 창별 키보드 입력 콜백 함수 포인터 타입
typedef void (*window_key_callback_t)(int key, void* user_data);

typedef struct {
    int id;
    int x, y, w, h;
    window_state_t state;
    int desktop_id;
    int is_dark_mode;
    char title[128];
    uint32_t *pixels;       // 이 창의 전용 픽셀 버퍼
    int pixels_dirty;       // 1이면 컴포지터가 이 창을 다시 그려야 함
    int is_focused;
    
    // [새로 추가] 이 창이 활성화되었을 때 호출될 콜백
    window_key_callback_t on_key_down;
    void *user_data;        // 콜백에 넘겨줄 앱의 컨텍스트 (예: 터미널 구조체 포인터)
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

#define WM_EVENT_QUEUE_SIZE 1024
typedef struct {
    wm_event_t events[WM_EVENT_QUEUE_SIZE];
    int head;
    int tail;
} wm_event_queue_t;

void wm_event_queue_init(wm_event_queue_t *q);
int wm_event_queue_push(wm_event_queue_t *q, wm_event_t *ev);
int wm_event_queue_pop(wm_event_queue_t *q, wm_event_t *out);

#endif
