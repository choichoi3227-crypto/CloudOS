// compositor.c
#include "wm_common.h"
#include "graphics.h"
#include "gpu.h"
#include <string.h>

// 백버퍼
static uint32_t backbuffer[SCREEN_W * SCREEN_H];

// damage 리스트 (최대 256개의 손상된 직사각형)
#define MAX_DAMAGE 256
typedef struct {
    int x, y, w, h;
} damage_rect_t;

static damage_rect_t damage_list[MAX_DAMAGE];
static int damage_count = 0;

static void add_damage(int x, int y, int w, int h) {
    if (damage_count >= MAX_DAMAGE) {
        // 꽉 차면 화면 전체를 damage로 처리
        damage_list[0].x = 0;
        damage_list[0].y = 0;
        damage_list[0].w = SCREEN_W;
        damage_list[0].h = SCREEN_H;
        damage_count = 1;
        return;
    }
    // 클램핑
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SCREEN_W) w = SCREEN_W - x;
    if (y + h > SCREEN_H) h = SCREEN_H - y;
    if (w <= 0 || h <= 0) return;

    damage_list[damage_count].x = x;
    damage_list[damage_count].y = y;
    damage_list[damage_count].w = w;
    damage_list[damage_count].h = h;
    damage_count++;
}

// 창을 백버퍼에 렌더링 (타이틀바 + 클라이언트 영역)
static void render_window_to_backbuffer(wm_window_t *w) {
    if (!w || w->id < 0 || w->state == WIN_STATE_MINIMIZED) return;
    // 클라이언트 영역 클리핑
    int cx = w->x, cy = w->y + TITLEBAR_H;
    int cw = w->w, ch = w->h - TITLEBAR_H;
    if (cw <= 0 || ch <= 0) return;

    // 클라이언트 영역 배경
    uint32_t client_bg = w->is_dark_mode ? 0xFF222222u : 0xFFFFFFFFu;
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H,
                                 cx, cy, cw, ch, client_bg);

    // 창 픽셀이 있으면 그대로 복사 (앱이 직접 그린 경우)
    if (w->pixels && w->pixels_dirty) {
        for (int row = 0; row < ch && (cy + row) < SCREEN_H; row++) {
            if (cy + row < 0) continue;
            int src_off = row * w->w;
            int dst_y = cy + row;
            int dst_off = dst_y * SCREEN_W + cx;
            int copy_w = (cw > w->w) ? w->w : cw;
            if (cx + copy_w > SCREEN_W) copy_w = SCREEN_W - cx;
            if (copy_w <= 0) continue;
            memcpy(&backbuffer[dst_off], &w->pixels[src_off], copy_w * sizeof(uint32_t));
        }
        w->pixels_dirty = 0;
    }

    // 타이틀바 배경
    uint32_t title_bg = w->is_dark_mode ? COLOR_TITLEBAR_DARK : COLOR_TITLEBAR_LIGHT;
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H,
                                 w->x, w->y, w->w, TITLEBAR_H, title_bg);

    // 타이틀 텍스트 (간단 예시)
    graphics_draw_string_on_buffer(backbuffer, SCREEN_W, SCREEN_H,
                                   w->title, w->x + 8, w->y + 6, COLOR_WHITE);

    // 닫기/최소화/최대화 버튼 (타이틀바 우측)
    int bx = w->x + w->w - 20;
    int by = w->y + 4;
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H,
                                 bx, by, 16, 16, 0xFFCC3333);
    // 버튼 텍스트 (X)
    graphics_draw_string_on_buffer(backbuffer, SCREEN_W, SCREEN_H,
                                   "X", bx + 3, by + 1, COLOR_WHITE);

    // TODO: 최소화/최대화 버튼 추가 가능
}

// WM에서 호출하는 메인 렌더 함수
void compositor_render_desktop(int desktop_id, wm_window_t *wins, int wins_count) {
    // 이번 프레임 damage 초기화
    damage_count = 0;

    // 1) 배경 (전체 화면 damage)
    add_damage(0, 0, SCREEN_W, SCREEN_H - TASKBAR_H);

    // 배경색: 데스크톱 ID에 따라 테마 전환 가능
    uint32_t bg = (desktop_id % 2 == 0) ? COLOR_DARK_BG : COLOR_LIGHT_BG;
    graphics_fill_rect_on_buffer(backbuffer, SCREEN_W, SCREEN_H,
                                 0, 0, SCREEN_W, SCREEN_H - TASKBAR_H, bg);

    // 2) 현재 데스크톱의 창만 렌더 (z-order 순서)
    for (int i = 0; i < wins_count; i++) {
        wm_window_t *w = &wins[i];
        if (w->id < 0 || w->desktop_id != desktop_id) continue;
        render_window_to_backbuffer(w);
    }

    // 3) 작업 표시줄 영역은 WM에서 직접 그리므로 여기서는 스킵 (선택)

    // 4) GPU 가속 블릭이 활성화되어 있으면 백버퍼→프론트버퍼를 GPU로 복사
    if (gpu_is_available()) {
        gpu_blit_to_framebuffer(backbuffer, SCREEN_W * SCREEN_H * sizeof(uint32_t));
    } else {
        // 소프트웨어 폴백: 백버퍼를 프론트버퍼로 복사
        graphics_copy_buffer_to_framebuffer(backbuffer, SCREEN_W, SCREEN_H);
    }
}
