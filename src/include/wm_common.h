// wm_common.h
#ifndef WM_COMMON_H
#define WM_COMMON_H

#include <stdint.h>
#include <stddef.h>

// 화면 (기본값, 나중에 모드 설정에서 가져오도록 개선)
#define SCREEN_W 1024
#define SCREEN_H 768

// 색상 (ARGB: 0xAARRGGBB, 그리기 함수에서 사용)
#define COLOR_TRANSPARENT 0x00000000u
#define COLOR_BLACK       0xFF000000u
#define COLOR_WHITE       0xFFFFFFFFu
#define COLOR_DARK_BG     0xFF1E1E2Eu // 다크 테마 배경
#define COLOR_LIGHT_BG    0xFFF0F0F0u // 라이트 테마 배경
#define COLOR_TITLEBAR_DARK  0xFF111111u
#define COLOR_TITLEBAR_LIGHT 0xFF333333u

// 창 상태
typedef enum {
    WIN_STATE_NORMAL = 0,
    WIN_STATE_MINIMIZED,
    WIN_STATE_MAXIMIZED,
    WIN_STATE_FULLSCREEN
} window_state_t;

// 창 속성
typedef struct {
    int id;                 // 창 ID (0 이상, -1이면 미사용)
    int x, y;               // 위치
    int w, h;               // 크기
    window_state_t state;   // 현재 상태
    int desktop_id;         // 소속 데스크톱 ID
    int is_dark_mode;       // 다크 모드 여부
    char title[128];        // 타이틀 텍스트

    // 렌더링용 버퍼 포인터 (이 창의 픽셀 데이터)
    uint32_t *pixels;       // (w * h) 픽셀, NULL이면 컴포지터가 타이틀바만 그림
    int pixels_dirty;       // 1이면 이 창의 pixels가 변경됨 (damage)
} wm_window_t;

// 입력 이벤트 타입
typedef enum {
    WM_EVENT_NONE = 0,
    WM_EVENT_MOUSE_MOVE,
    WM_EVENT_MOUSE_DOWN,
    WM_EVENT_MOUSE_UP,
    WM_EVENT_MOUSE_WHEEL,
    WM_EVENT_KEY_DOWN,
    WM_EVENT_KEY_UP,
    WM_EVENT_WINDOW_CLOSE,     // 타이틀바 X 버튼 등
    WM_EVENT_WINDOW_SNAP_LEFT,
    WM_EVENT_WINDOW_SNAP_RIGHT,
    WM_EVENT_WINDOW_MAXIMIZE,
    WM_EVENT_WINDOW_MINIMIZE,
    WM_EVENT_DESKTOP_SWITCH,   // 가상 데스크톱 전환 요청
    WM_EVENT_APP_LAUNCH        // 앱 실행 요청 (파라미터로 경로 등)
} wm_event_type_t;

// 입력 이벤트 구조체
typedef struct {
    wm_event_type_t type;
    int x, y;           // 마우스 위치 (절대 좌표)
    int dx, dy;         // 마우스 이동 델타 (드래그 계산용)
    int button;         // 마우스 버튼 (1=left, 2=right, 3=middle)
    int key;            // 키 코드
    int window_id;      // 이벤트가 속한 창 ID (음수면 전역)
    int desktop_id;     // 대상 데스크톱 ID (데스크톱 전환용)
    char app_path[256]; // APP_LAUNCH 시 실행 파일 경로
} wm_event_t;

// 창 관련 옵션/플래그
#define WM_MAX_WINDOWS 64
#define TASKBAR_H 32            // 하단 작업 표시줄 높이
#define TITLEBAR_H 24           // 타이틀바 높이
#define SNAP_LEFT_W  (SCREEN_W / 2)
#define SNAP_RIGHT_X (SCREEN_W / 2)
#define SNAP_RIGHT_W (SCREEN_W / 2)
#define MIN_WIN_W 120
#define MIN_WIN_H 80

// 데스크톱 수 (초기)
#define WM_MAX_DESKTOPS 4

#endif // WM_COMMON_H
