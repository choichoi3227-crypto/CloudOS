// wm.c
#include "wm_common.h"
#include "graphics.h"
#include "compositor.h"
#include "keyboard.h"
#include "mouse.h"
#include <string.h>

// 전역 상태
static wm_window_t windows[WM_MAX_WINDOWS];
static int window_count = 0;
static int next_window_id = 1;

static int current_desktop = 0;
static int mouse_x = 0, mouse_y = 0;
static int drag_window_id = -1;
static int drag_start_mx = 0, drag_start_my = 0;
static int drag_start_wx = 0, drag_start_wy = 0;

static int resize_window_id = -1;
static int resize_edge = 0; // 비트: 1=left, 2=top, 4=right, 8=bottom
static int resize_start_mx = 0, resize_start_my = 0;
static int resize_start_wx = 0, resize_start_wy = 0;
static int resize_start_ww = 0, resize_start_wh = 0;

// 내부 유틸
static int find_window_by_id(int id) {
    if (id < 0 || id >= WM_MAX_WINDOWS) return -1;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (windows[i].id == id) return i;
    }
    return -1;
}

static wm_window_t *window_by_id(int id) {
    int idx = find_window_by_id(id);
    return (idx >= 0) ? &windows[idx] : NULL;
}

// 새 창 생성 (앱에서 호출)
int wm_create_window(const char *title, int x, int y, int w, int h, int desktop_id, int is_dark_mode) {
    int slot = -1;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        if (windows[i].id < 0) { slot = i; break; }
    }
    if (slot < 0) return -1; // 자리 없음

    wm_window_t *w = &windows[slot];
    w->id = next_window_id++;
    w->x = x; w->y = y;
    w->w = w; w->h = h;
    w->state = WIN_STATE_NORMAL;
    w->desktop_id = desktop_id;
    w->is_dark_mode = is_dark_mode;
    w->pixels = NULL;
    w->pixels_dirty = 0;
    if (title) {
        strncpy(w->title, title, sizeof(w->title) - 1);
        w->title[sizeof(w->title) - 1] = '\0';
    } else {
        w->title[0] = '\0';
    }

    window_count++;
    return w->id;
}

// 창 닫기
int wm_close_window(int id) {
    wm_window_t *w = window_by_id(id);
    if (!w) return -1;
    if (w->pixels) { /* TODO: w->pixels 해제 */ w->pixels = NULL; }
    w->id = -1;
    window_count--;
    return 0;
}

// 창 속성 업데이트 (크기/위치/제목/테마)
int wm_update_window(int id, int x, int y, int w, int h, const char *title, int is_dark_mode) {
    wm_window_t *win = window_by_id(id);
    if (!win) return -1;
    if (x >= 0) win->x = x;
    if (y >= 0) win->y = y;
    if (w >= 0) win->w = w;
    if (h >= 0) win->h = h;
    if (title) {
        strncpy(win->title, title, sizeof(win->title) - 1);
        win->title[sizeof(win->title) - 1] = '\0';
    }
    if (is_dark_mode >= 0) win->is_dark_mode = is_dark_mode;
    return 0;
}

// 포인트가 창 안에 있는지
static int point_in_window(wm_window_t *w, int px, int py) {
    return (px >= w->x && px < w->x + w->w &&
            py >= w->y && py < w->y + w->h);
}

// 타이틀바 영역인지 (y < w->y + TITLEBAR_H)
static int point_in_titlebar(wm_window_t *w, int px, int py) {
    return (px >= w->x && px < w->x + w->w &&
            py >= w->y && py < w->y + TITLEBAR_H);
}

// 특정 좌표의 최상위 창 ID (z-order 마지막이 최상위)
static int top_window_at(int x, int y) {
    for (int i = WM_MAX_WINDOWS - 1; i >= 0; i--) {
        if (windows[i].id >= 0 &&
            windows[i].desktop_id == current_desktop &&
            windows[i].state != WIN_STATE_MINIMIZED &&
            point_in_window(&windows[i], x, y)) {
            return windows[i].id;
        }
    }
    return -1;
}

// Z-order 맨 위로 올리기
static void raise_window(int id) {
    int idx = find_window_by_id(id);
    if (idx < 0) return;
    wm_window_t tmp = windows[idx];
    for (int i = idx; i < WM_MAX_WINDOWS - 1; i++) {
        windows[i] = windows[i + 1];
    }
    windows[WM_MAX_WINDOWS - 1] = tmp;
}

// 스냅 로직
static void snap_window(int id, int side) {
    wm_window_t *w = window_by_id(id);
    if (!w) return;
    if (side == 0) { // LEFT
        w->x = 0;
        w->y = TITLEBAR_H;
        w->w = SNAP_LEFT_W;
        w->h = SCREEN_H - TITLEBAR_H - TASKBAR_H;
        w->state = WIN_STATE_NORMAL;
    } else if (side == 1) { // RIGHT
        w->x = SNAP_RIGHT_X;
        w->y = TITLEBAR_H;
        w->w = SNAP_RIGHT_W;
        w->h = SCREEN_H - TITLEBAR_H - TASKBAR_H;
        w->state = WIN_STATE_NORMAL;
    } else if (side == 2) { // MAXIMIZE
        w->x = 0;
        w->y = TITLEBAR_H;
        w->w = SCREEN_W;
        w->h = SCREEN_H - TITLEBAR_H - TASKBAR_H;
        w->state = WIN_STATE_MAXIMIZED;
    }
}

// 리사이즈 엣지 판단 (창 경계 근처면 해당 엣지 비트 설정)
static int calc_resize_edge(wm_window_t *w, int px, int py) {
    int edge = 0;
    int border = 6; // 엣지 감지 두께
    int in = point_in_window(w, px, py);
    if (!in) return edge;
    if (px < w->x + border) edge |= 1;  // left
    if (py < w->y + border) edge |= 2;  // top
    if (px >= w->x + w->w - border) edge |= 4;  // right
    if (py >= w->y + w->h - border) edge |= 8;  // bottom
    return edge;
}

// 이벤트 처리 핸들러들
static void handle_mouse_down(wm_event_t *ev) {
    int wid = top_window_at(ev->x, ev->y);
    if (wid < 0) return;
    raise_window(wid);
    wm_window_t *w = window_by_id(wid);
    if (!w) return;

    if (ev->button == 1) {
        // 타이틀바 클릭 -> 드래그 시작 또는 더블클릭 최대화
        if (point_in_titlebar(w, ev->x, ev->y)) {
            // TODO: 더블클릭 감지해서 최대화 토글 구현 가능
            drag_window_id = wid;
            drag_start_mx = ev->x;
            drag_start_my = ev->y;
            drag_start_wx = w->x;
            drag_start_wy = w->y;
            return;
        }
        // 엣지 근처 클릭 -> 리사이즈 시작
        int edge = calc_resize_edge(w, ev->x, ev->y);
        if (edge != 0) {
            resize_window_id = wid;
            resize_edge = edge;
            resize_start_mx = ev->x;
            resize_start_my = ev->y;
            resize_start_wx = w->x;
            resize_start_wy = w->y;
            resize_start_ww = w->w;
            resize_start_wh = w->h;
            return;
        }
    }
    // 우클릭 등은 컨텍스트 메뉴용 (TODO)
}

static void handle_mouse_up(wm_event_t *ev) {
    if (ev->button == 1) {
        if (drag_window_id >= 0) {
            // 드래그 끝
            drag_window_id = -1;
        }
        if (resize_window_id >= 0) {
            // 리사이즈 끝
            resize_window_id = -1;
            resize_edge = 0;
        }
    }
}

static void handle_mouse_move(wm_event_t *ev) {
    mouse_x = ev->x;
    mouse_y = ev->y;
    int dx = ev->dx;
    int dy = ev->dy;

    if (drag_window_id >= 0) {
        wm_window_t *w = window_by_id(drag_window_id);
        if (w && w->state != WIN_STATE_MAXIMIZED) {
            w->x = drag_start_wx + (ev->x - drag_start_mx);
            w->y = drag_start_wy + (ev->y - drag_start_my);
        }
        return;
    }
    if (resize_window_id >= 0) {
        wm_window_t *w = window_by_id(resize_window_id);
        if (!w) return;
        int new_x = resize_start_wx;
        int new_y = resize_start_wy;
        int new_w = resize_start_ww;
        int new_h = resize_start_wh;

        if (resize_edge & 1) { // left
            int delta = ev->x - resize_start_mx;
            new_x = resize_start_wx + delta;
            new_w = resize_start_ww - delta;
        }
        if (resize_edge & 4) { // right
            int delta = ev->x - resize_start_mx;
            new_w = resize_start_ww + delta;
        }
        if (resize_edge & 2) { // top
            int delta = ev->y - resize_start_my;
            new_y = resize_start_wy + delta;
            new_h = resize_start_wh - delta;
        }
        if (resize_edge & 8) { // bottom
            int delta = ev->y - resize_start_my;
            new_h = resize_start_wh + delta;
        }

        // 최소 크기 보장
        if (new_w < MIN_WIN_W) new_w = MIN_WIN_W;
        if (new_h < MIN_WIN_H) new_h = MIN_WIN_H;

        w->x = new_x;
        w->y = new_y;
        w->w = new_w;
        w->h = new_h;
    }
}

static void handle_key_down(wm_event_t *ev) {
    // Alt+Tab: 창 전환 (간단 예시)
    if (ev->key == 0x0F && (ev->key & 0x100)) { // 임의 키 코드 매핑; 실제 키보드 드라이버에 맞춤
        // TODO: 실제 Alt+Tab 조합 확인 방식에 맞게 수정
        // 현재 데스크톱의 창들을 순환하며 활성화
        int first = -1, prev = -1, active = -1;
        for (int i = 0; i < WM_MAX_WINDOWS; i++) {
            wm_window_t *w = &windows[i];
            if (w->id < 0 || w->desktop_id != current_desktop) continue;
            if (w->state == WIN_STATE_MINIMIZED) continue;
            if (first < 0) first = w->id;
            prev = active;
            active = w->id;
        }
        if (first >= 0 && active >= 0) {
            int next = (prev >= 0) ? prev : first;
            raise_window(next);
        }
    }
}

// 작업 표시줄 그리기
static void draw_taskbar(void) {
    int y0 = SCREEN_H - TASKBAR_H;
    uint32_t bg = COLOR_LIGHT_BG;
    graphics_fill_rect(0, y0, SCREEN_W, TASKBAR_H, bg);

    // 간단한 시작 버튼 (좌측)
    graphics_fill_rect(0, y0, 48, TASKBAR_H, 0xFF2D7D46);
    graphics_draw_string("Start", 4, y0 + 8, COLOR_WHITE);

    // 현재 데스크톱의 창 목록 (간단 타이틀 표시)
    int xoff = 52;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        wm_window_t *w = &windows[i];
        if (w->id < 0 || w->desktop_id != current_desktop) continue;
        if (w->state == WIN_STATE_MINIMIZED) continue;
        // 버튼 형태로 타이틀 표시
        int tw = graphics_measure_string(w->title) + 16;
        if (tw < 60) tw = 60;
        if (xoff + tw > SCREEN_W - 8) break; // 오버플로 방지
        graphics_fill_rect(xoff, y0 + 2, tw, TASKBAR_H - 4, 0xFF222222);
        graphics_draw_string(w->title, xoff + 4, y0 + 8, COLOR_WHITE);
        xoff += tw + 2;
    }

    // 시계 (우측) – TODO: 실제 시간 가져오기
    graphics_draw_string("00:00", SCREEN_W - 40, y0 + 8, COLOR_BLACK);
}

// WM 초기화
void wm_init(void) {
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        windows[i].id = -1;
        windows[i].pixels = NULL;
    }
    window_count = 0;
    current_desktop = 0;
    drag_window_id = -1;
    resize_window_id = -1;
}

// WM 메인 루프 (커널 메인에서 호출)
void wm_run(void) {
    wm_init();

    // 테스트용 창 하나 생성
    wm_create_window("CloudOS Terminal", 40, 60, 640, 480, current_desktop, 0);

    while (1) {
        // 입력 이벤트 수신 (키보드/마우스 드라이버에서 큐/버퍼로 읽어온다고 가정)
        wm_event_t ev = {0};
        // TODO: 실제 드라이버에서 읽어오는 부분은 각자 구현
        // 예: keyboard_poll_event(&ev); mouse_poll_event(&ev);
        // 아래는 예시로 마우스 이벤트만 가정
        ev.type = WM_EVENT_MOUSE_MOVE;
        ev.x = mouse_x;
        ev.y = mouse_y;
        // ev.dx, ev.dy, ev.button, ev.key 등은 드라이버가 채움

        if (ev.type == WM_EVENT_MOUSE_DOWN) {
            handle_mouse_down(&ev);
        } else if (ev.type == WM_EVENT_MOUSE_UP) {
            handle_mouse_up(&ev);
        } else if (ev.type == WM_EVENT_MOUSE_MOVE) {
            handle_mouse_move(&ev);
        } else if (ev.type == WM_EVENT_KEY_DOWN) {
            handle_key_down(&ev);
        } else if (ev.type == WM_EVENT_WINDOW_SNAP_LEFT) {
            int wid = ev.window_id;
            if (wid < 0) wid = top_window_at(mouse_x, mouse_y);
            snap_window(wid, 0);
        } else if (ev.type == WM_EVENT_WINDOW_SNAP_RIGHT) {
            int wid = ev.window_id;
            if (wid < 0) wid = top_window_at(mouse_x, mouse_y);
            snap_window(wid, 1);
        } else if (ev.type == WM_EVENT_WINDOW_MAXIMIZE) {
            int wid = ev.window_id;
            if (wid < 0) wid = top_window_at(mouse_x, mouse_y);
            snap_window(wid, 2);
        } else if (ev.type == WM_EVENT_DESKTOP_SWITCH) {
            int new_desk = ev.desktop_id;
            if (new_desk >= 0 && new_desk < WM_MAX_DESKTOPS) {
                current_desktop = new_desk;
            }
        }

        // 컴포지터에게 렌더링 지시 (damage만 갱신)
        compositor_render_desktop(current_desktop, &windows[0], WM_MAX_WINDOWS);

        // 작업 표시줄은 매 프레임 그림 (단순화)
        draw_taskbar();

        // 프레임 버퍼 스왑 (더블버퍼링은 compositor.c에서 담당)
        graphics_swap_buffers();

        // 유휴 대기 (타이머/인터럽트 기반 sleep)
        // schedule_yield(); // 스케줄러가 있으면 호출
    }
}
