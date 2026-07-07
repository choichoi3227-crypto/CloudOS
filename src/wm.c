// wm.c
#include "wm.h"
#include "graphics.h"
#include "compositor.h"
#include "string.h"

static wm_window_t windows[WM_MAX_WINDOWS];
static int window_count = 0;
static int next_window_id = 1;
static int current_desktop = 0;
static int mouse_x = SCREEN_W / 2, mouse_y = SCREEN_H / 2;
static int active_window_id = -1;

// 상용 OS 동시성 제어: 인터럽트 컨텍스트에서 안전한 Lock-Free 이벤트 큐
static wm_event_queue_t input_queue = {0};

void wm_event_queue_init(wm_event_queue_t *q) {
    q->head = 0; q->tail = 0;
}

int wm_event_queue_push(wm_event_queue_t *q, wm_event_t *ev) {
    int next = (q->head + 1) % WM_EVENT_QUEUE_SIZE;
    if (next == q->tail) return 0; // Queue Full
    q->events[q->head] = *ev;
    q->head = next;
    return 1;
}

int wm_event_queue_pop(wm_event_queue_t *q, wm_event_t *out) {
    if (q->head == q->tail) return 0; // Queue Empty
    *out = q->events[q->tail];
    q->tail = (q->tail + 1) % WM_EVENT_QUEUE_SIZE;
    return 1;
}

// 퍼블릭 인터페이스 (mouse.c / keyboard.c 인터럽트 핸들러에서 호출용)
void wm_push_input_event(wm_event_t *ev) {
    wm_event_queue_push(&input_queue, ev);
}

// 윈도우 매니저 로컬 상태
static int drag_win_id = -1, resize_win_id = -1;
static int drag_sx, drag_sy, drag_wx, drag_wy;
static int resize_edge, resize_sx, resize_sy, resize_wx, resize_wy, resize_ww, resize_wh;

static wm_window_t *get_window(int id) {
    for (int i = 0; i < WM_MAX_WINDOWS; i++) if (windows[i].id == id) return &windows[i];
    return NULL;
}

static void raise_window(int id) {
    wm_window_t tmp = {0}; int idx = -1;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) if (windows[i].id == id) { idx = i; break; }
    if (idx < 0) return;
    tmp = windows[idx];
    for (int i = idx; i < WM_MAX_WINDOWS - 1; i++) windows[i] = windows[i + 1];
    windows[WM_MAX_WINDOWS - 1] = tmp;
    active_window_id = id;
}

int wm_create_window(const char *title, int x, int y, int w, int h, int desktop_id, int is_dark_mode) {
    int slot = -1;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) if (windows[i].id < 0) { slot = i; break; }
    if (slot < 0) return -1;
    wm_window_t *win = &windows[slot];
    win->id = next_window_id++;
    win->x = x; win->y = y; win->w = w; win->h = h;
    win->state = WIN_STATE_NORMAL; win->desktop_id = desktop_id; win->is_dark_mode = is_dark_mode;
    win->pixels = NULL; win->pixels_dirty = 0; win->is_focused = 0;
    if (title) { strncpy(win->title, title, 127); win->title[127] = '\0'; }
    window_count++;
    raise_window(win->id); // 생성 시 자동적으로 맨 위로
    return win->id;
}

int wm_close_window(int id) {
    wm_window_t *w = get_window(id);
    if (!w) return -1;
    w->id = -1; window_count--;
    if (active_window_id == id) active_window_id = -1;
    return 0;
}

static int point_in_rect(int px, int py, int rx, int ry, int rw, int rh) {
    return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

static int top_window_at(int x, int y) {
    for (int i = WM_MAX_WINDOWS - 1; i >= 0; i--) {
        wm_window_t *w = &windows[i];
        if (w->id > 0 && w->desktop_id == current_desktop && w->state != WIN_STATE_MINIMIZED) {
            if (point_in_rect(x, y, w->x, w->y, w->w, w->h)) return w->id;
        }
    }
    return -1;
}

// 상용 OS 마우스 커서 렌더링 (하드웨어 커서 없을 때 소프트웨어로 그림)
static void draw_mouse_cursor(void) {
    uint32_t c = COLOR_WHITE;
    draw_pixel(mouse_x, mouse_y, c);
    draw_pixel(mouse_x+1, mouse_y, c); draw_pixel(mouse_x, mouse_y+1, c);
    draw_pixel(mouse_x+2, mouse_y, c); draw_pixel(mouse_x, mouse_y+2, c);
}

// 상용 OS 작업표시줄 렌더링 (시계 포함)
static void draw_taskbar(void) {
    int y0 = SCREEN_H - TASKBAR_H;
    draw_rect(0, y0, SCREEN_W, TASKBAR_H, COLOR_TASKBAR_BG);

    // 시작 버튼
    draw_rect(0, y0, 48, TASKBAR_H, COLOR_TITLEBAR_ACT);
    draw_string("Start", 6, y0 + 14, COLOR_TEXT_WHITE);

    // 창 버튼 목록
    int xoff = 60;
    for (int i = 0; i < WM_MAX_WINDOWS; i++) {
        wm_window_t *w = &windows[i];
        if (w->id < 0 || w->desktop_id != current_desktop || w->state == WIN_STATE_MINIMIZED) continue;
        int tw = graphics_measure_string(w->title) + 20;
        if (tw < 100) tw = 100;
        if (xoff + tw > SCREEN_W - 100) break;
        
        uint32_t btn_bg = (w->id == active_window_id) ? COLOR_TITLEBAR_ACT : COLOR_TASKBAR_BTN;
        draw_rect(xoff, y0 + 4, tw, TASKBAR_H - 8, btn_bg);
        draw_string(w->title, xoff + 10, y0 + 14, COLOR_TEXT_WHITE);
        xoff += tw + 4;
    }

    // 실시간 하드웨어 시계 (RTC)
    rtc_time_t now;
    read_rtc(&now);
    char time_str[9];
    time_str[0] = '0' + (now.hour / 10); time_str[1] = '0' + (now.hour % 10); time_str[2] = ':';
    time_str[3] = '0' + (now.minute / 10); time_str[4] = '0' + (now.minute % 10); time_str[5] = ':';
    time_str[6] = '0' + (now.second / 10); time_str[7] = '0' + (now.second % 10); time_str[8] = '\0';
    draw_string(time_str, SCREEN_W - 80, y0 + 14, COLOR_TEXT_WHITE);
}

// 이벤트 처리 로직
static void process_event(wm_event_t *ev) {
    if (ev->type == WM_EVENT_MOUSE_MOVE) {
        mouse_x = ev->x; mouse_y = ev->y;
        if (drag_win_id > 0) {
            wm_window_t *w = get_window(drag_win_id);
            if (w && w->state != WIN_STATE_MAXIMIZED) {
                w->x = drag_wx + (ev->x - drag_sx);
                w->y = drag_wy + (ev->y - drag_sy);
                if (w->y < 0) w->y = 0; // 화면 밖으로 나가지 않게 제한
            }
        } else if (resize_win_id > 0) {
            wm_window_t *w = get_window(resize_win_id);
            if (w) {
                int nx = resize_wx, ny = resize_wy, nw = resize_ww, nh = resize_wh;
                if (resize_edge & 1) { nx += ev->x - resize_sx; nw -= ev->x - resize_sx; }
                if (resize_edge & 2) { ny += ev->y - resize_sy; nh -= ev->y - resize_sy; }
                if (resize_edge & 4) { nw += ev->x - resize_sx; }
                if (resize_edge & 8) { nh += ev->y - resize_sy; }
                if (nw < MIN_WIN_W) nw = MIN_WIN_W; if (nh < MIN_WIN_H) nh = MIN_WIN_H;
                w->x = nx; w->y = ny; w->w = nw; w->h = nh;
            }
        }
    } 
    else if (ev->type == WM_EVENT_MOUSE_DOWN && ev->button == 1) {
        int wid = top_window_at(ev->x, ev->y);
        if (wid > 0) {
            raise_window(wid);
            wm_window_t *w = get_window(wid);
            
            // 닫기 버튼 히트 테스트
            int close_x = w->x + w->w - 20;
            int btn_y = w->y + 8;
            if (point_in_rect(ev->x, ev->y, close_x, btn_y, 16, 16)) {
                wm_close_window(wid); return;
            }
            // 최대화 버튼 히트 테스트
            int max_x = close_x - 20;
            if (point_in_rect(ev->x, ev->y, max_x, btn_y, 16, 16)) {
                compositor_snap_window(w, 2); return;
            }
            // 타이틀바 드래그 시작
            if (point_in_rect(ev->x, ev->y, w->x, w->y, w->w, TITLEBAR_H)) {
                drag_win_id = wid; drag_sx = ev->x; drag_sy = ev->y; drag_wx = w->x; drag_wy = w->y;
                return;
            }
            // 엣지 리사이즈 시작 (하단 우측 모서리 10px)
            if (ev->x > w->x + w->w - 10 && ev->y > w->y + w->h - 10) {
                resize_win_id = wid; resize_edge = 12; // 하단+우측
                resize_sx = ev->x; resize_sy = ev->y; resize_wx = w->x; resize_wy = w->y; resize_ww = w->w; resize_wh = w->h;
            }
        }
    }
    else if (ev->type == WM_EVENT_MOUSE_UP && ev->button == 1) {
        drag_win_id = -1; resize_win_id = -1; resize_edge = 0;
    }
}

void wm_init(void) {
    wm_event_queue_init(&input_queue);
    for (int i = 0; i < WM_MAX_WINDOWS; i++) windows[i].id = -1;
}

void wm_run(void) {
    wm_init();
    compositor_init();

    // 테스트용 창 2개 생성
    wm_create_window("CloudOS Terminal", 50, 50, 600, 400, 0, 1);
    wm_create_window("File Manager", 150, 120, 500, 350, 0, 0);

    wm_event_t ev;
    // 상용 OS 메인 이벤트 루프 (유휴 시 대기하며 이벤트를 기다림)
    while (1) {
        // 1. 큐에서 이벤트를 모두 꺼내서 처리
        while (wm_event_queue_pop(&input_queue, &ev)) {
            process_event(&ev);
        }

        // 2. 컴포지터가 백버퍼에 프레임 구성
        compositor_render_desktop(current_desktop, windows, WM_MAX_WINDOWS);

        // 3. 프론트버퍼 위에 작업표시줄과 마우스 커서 직접 렌더링 (Compositing 우선순위 최상단)
        draw_taskbar();
        draw_mouse_cursor();

        // 4. 유휴 상태 진입 (CPU 점유율 100% 방지, 상용 OS 필수)
        // 실제 구현 시에는 asm volatile("hlt"); 를 사용하여 인터럽트 대기
        asm volatile("hlt");
    }
}
