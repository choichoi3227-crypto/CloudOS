#include "compositor.h"
#include "graphics.h"
#include "string.h"

static compositor_window_t windows[MAX_WINDOWS];
static int window_count = 0;

void compositor_init(void) {
    for(int i=0; i<MAX_WINDOWS; i++) windows[i].is_open = 0;
}

void compositor_switch_desktop(int new_desktop) {
    // 가상 데스크탑 전환 애니메이션 처리 (생략)
}

void compositor_snap_window(int win_id, int side) {
    compositor_window_t* w = &windows[win_id];
    if (side == 0) { // Left Snap
        w->x = 0; w->y = 30; w->w = 512; w->h = 738;
    } else if (side == 1) { // Right Snap
        w->x = 512; w->y = 30; w->w = 512; w->h = 738;
    } else if (side == 2) { // Maximize
        w->x = 0; w->y = 30; w->w = 1024; w->h = 738;
    }
}

void compositor_render(int current_desktop) {
    // 배경화면 (동적 배경화면 지원 기반)
    if (current_desktop % 2 == 0) {
        draw_rect(0, 0, 1024, 768, 0x1E1E2E); // Dark
    } else {
        draw_rect(0, 0, 1024, 768, 0xF0F0F0); // Light
    }

    // 현재 데스크탑의 창만 렌더링
    for (int i = 0; i < window_count; i++) {
        if (windows[i].is_open && windows[i].desktop_id == current_desktop) {
            // 다크모드 창은 테두리 색상 변경
            uint32_t title_color = windows[i].is_dark_mode ? 0x111111 : 0x333333;
            uint32_t bg_color = windows[i].is_dark_mode ? 0x222222 : 0xFFFFFF;
            
            draw_rect(windows[i].x, windows[i].y, windows[i].w, windows[i].h, bg_color);
            draw_rect(windows[i].x, windows[i].y, windows[i].w, 25, title_color);
            draw_string(windows[i].title, windows[i].x + 10, windows[i].y + 5, 0xFFFFFF);
        }
    }
}
