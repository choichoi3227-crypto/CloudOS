// compositor.c
#include "compositor.h"
#include "graphics.h"
#include "gpu.h"
#include "heap.h" // 레포에 있는 힙 할당자 사용
#include "string.h"

static uint32_t *backbuffer = NULL;

void compositor_init(void) {
    // 상용 OS 방식: 정적 배열 대신 커널 힙에서 동적 할당 (OOM 방지)
    backbuffer = (uint32_t *)kmalloc(SCREEN_W * SCREEN_H * sizeof(uint32_t));
    if (!backbuffer) {
        // 치명적 오류 처리 (무한 루프로 패닉 유도)
        for(;;) { outb(0x64, 0xFE); } 
    }
}

// 상용 OS 핵심 로직: 완벽한 직사각형 클리핑
typedef struct { int x1, y1, x2, y2; } clip_rect_t;
static void get_clip_for_window(wm_window_t *w, clip_rect_t *clip) {
    clip->x1 = w->x < 0 ? 0 : w->x;
    clip->y1 = w->y < 0 ? 0 : w->y;
    clip->x2 = (w->x + w->w) > SCREEN_W ? SCREEN_W : (w->x + w->w);
    clip->y2 = (w->y + w->h) > SCREEN_H ? SCREEN_H : (w->y + w->h);
}

// 상용 OS 디자인: 창 뒤에 미세한 그림자(Shadow) 렌더링
static void render_shadow(wm_window_t *w) {
    int offset = 4; // 그림자 깊이
    uint32_t shadow_color = 0x20000000; // 약간 투명한 검은색
    for (int i = 0; i < offset; i++) {
        graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 
                                    w->x + offset + i, w->y + offset + i, w->w, w->h, shadow_color);
    }
}

static void render_window(wm_window_t *w, int is_top_window) {
    if (!w || w->id < 0 || w->state == WIN_STATE_MINIMIZED) return;
    clip_rect_t clip;
    get_clip_for_window(w, &clip);
    if (clip.x1 >= clip.x2 || clip.y1 >= clip.y2) return;

    // 1. 그림자 렌더링 (최상단 창이 아닌 경우에만)
    if (!is_top_window) render_shadow(w);

    // 2. 타이틀바 색상 결정 (포커스 여부에 따라 액센트 컬러 변경)
    uint32_t title_color = w->is_focused ? COLOR_TITLEBAR_ACT : COLOR_TITLEBAR_INC;
    uint32_t client_color = w->is_dark_mode ? COLOR_CLIENT_DARK : COLOR_CLIENT_LIGHT;

    // 3. 클라이언트 영역
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 
                                w->x, w->y + TITLEBAR_H, w->w, w->h - TITLEBAR_H, client_color);

    // 4. 타이틀바
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 
                                w->x, w->y, w->w, TITLEBAR_H, title_color);

    // 5. 타이틀 텍스트
    graphics_draw_string_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 
                                  w->title, w->x + 10, w->y + 10, COLOR_TEXT_WHITE);

    // 6. 타이틀바 버튼들 (우측 정렬, 마우스 클릭 영역 확보)
    int btn_y = w->y + 8;
    int btn_size = 16;
    int btn_margin = 4;
    
    // 닫기 버튼 (빨간색)
    int close_x = w->x + w->w - (btn_size + btn_margin);
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 
                                close_x, btn_y, btn_size, btn_size, COLOR_BTN_CLOSE);
    graphics_draw_string_on_buffer(backbuffer, SCREEN_W, SCREEN_H, "X", close_x + 4, btn_y, COLOR_TEXT_WHITE);

    // 최대화 버튼
    int max_x = close_x - (btn_size + btn_margin);
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 
                                max_x, btn_y, btn_size, btn_size, title_color);
    graphics_draw_string_on_buffer(backbuffer, SCREEN_W, SCREEN_H, "[", max_x + 4, btn_y, COLOR_TEXT_WHITE);
}

void compositor_switch_desktop(int new_desktop) { (void)new_desktop; }

void compositor_snap_window(wm_window_t *w, int side) {
    if (!w) return;
    if (side == 0) { w->x = 0; w->y = 0; w->w = SCREEN_W / 2; w->h = SCREEN_H - TASKBAR_H; }
    else if (side == 1) { w->x = SCREEN_W / 2; w->y = 0; w->w = SCREEN_W / 2; w->h = SCREEN_H - TASKBAR_H; }
    else if (side == 2) { w->x = 0; w->y = 0; w->w = SCREEN_W; w->h = SCREEN_H - TASKBAR_H; w->state = WIN_STATE_MAXIMIZED; }
}

void compositor_render_desktop(int desktop_id, wm_window_t *windows, int windows_count) {
    // 배경 렌더링
    uint32_t bg = (desktop_id % 2 == 0) ? COLOR_DARK_BG : COLOR_LIGHT_BG;
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 0, 0, SCREEN_W, SCREEN_H - TASKBAR_H, bg);

    // Z-Order 기반 창 합성 (맨 뒤창부터 그려서 앞창이 덮이게 함)
    int top_id = -1;
    for (int i = 0; i < windows_count; i++) {
        if (windows[i].id == windows_count - 1) top_id = windows[i].id; // 단순화된 포커스 추적
    }

    for (int i = 0; i < windows_count; i++) {
        wm_window_t *w = &windows[i];
        if (w->id < 0 || w->desktop_id != desktop_id) continue;
        w->is_focused = (w->id == top_id) ? 1 : 0;
        render_window(w, w->is_focused);
    }

    // 작업표시줄 배경
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H, 0, SCREEN_H - TASKBAR_H, SCREEN_W, TASKBAR_H, COLOR_TASKBAR_BG);

    // 프론트버퍼로 스왑
    if (gpu_is_available()) gpu_blit_to_framebuffer(backbuffer, SCREEN_W * SCREEN_H * 4);
    else graphics_copy_buffer_to_framebuffer(backbuffer, SCREEN_W, SCREEN_H);
}
