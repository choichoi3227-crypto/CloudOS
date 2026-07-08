// src/wm.c (기존 파일 덮어쓰기 - 핵심 부분 수정)
#include "wm.h"
#include "graphics.h"
#include "compositor.h"
#include "keyboard.h"
#include "mouse.h"
#include "apps/terminal.h"
#include "fs/vfs.h"
#include "string.h"

// ... (이전 변수들 동일: windows[], mouse_x, input_queue 등) ...

int wm_create_window(const char *title, int x, int y, int w, int h, int desktop_id, int is_dark_mode) {
    // ... (이전 로직 동일) ...
    win->on_key_down = NULL;
    win->user_data = NULL;
    // ... (이전 로직 동일) ...
}

void wm_run(void) {
    wm_init();
    compositor_init();
    vfs_init(); // [추가] 가상 파일 시스템 초기화
    
    // [추가] 기본 디렉토리와 파일 생성
    vfs_create_dir("/bin");
    vfs_create_dir("/sys");
    vfs_create_dir("/apps");
    vfs_create_file("/bin/hello", "Hello from CloudOS VFS!");
    vfs_create_file("/sys/info", "CloudOS Kernel v1.0");

    // [수정] 터미널 앱 생성 (이전 테스트 창 2개 대신 진짜 터미널 1개 띄우기)
    int term_id = wm_create_window("CloudOS Terminal", 50, 50, 640, 480, 0, 1);
    wm_window_t *term_win = get_window(term_id);
    
    if (term_win) {
        terminal_init(term_win); // 터미널 내부 버퍼 할당 및 초기 화면 구성
        terminal_draw(term_win);   // 초기 텍스트를 픽셀에 그림
        raise_window(term_id);  // 맨 위로 올림
    }

    wm_event_t ev;
    while (1) {
        while (wm_event_queue_pop(&input_queue, &ev)) {
            process_event(&ev);
            
            // [핵심 추가] 키 이벤트 발생 시 활성 창의 콜백 호출
            if (ev.type == WM_EVENT_KEY_DOWN && active_window_id > 0) {
                wm_window_t *active_win = get_window(active_window_id);
                if (active_win && active_win->on_key_down) {
                    active_win->on_key_down(ev.key, active_win->user_data);
                    // 앱(터미널)이 내부적으로 픽셀를 수정했으니 컴포지터에게 다시 그리라고 플래그 세팅
                    active_win->pixels_dirty = 1; 
                }
            }
        }

        compositor_render_desktop(current_desktop, windows, WM_MAX_WINDOWS);
        draw_taskbar();
        draw_mouse_cursor();
        asm volatile("hlt");
    }
}
// ... (나머지 코드 동일) ...
