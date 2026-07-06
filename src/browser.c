#include "browser.h"
#include "graphics.h"
#include "vga.h"

// 네이버나 크롬과 같은 브라우저 UI를 창 내부에 렌더링
void browser_render_page(int win_x, int win_y, int win_w, int win_h) {
    // 1. 브라우저 상단 도구 모음 (주소창)
    draw_rect(win_x, win_y + 25, win_w, 30, 0xEEEEEE); // 배경
    draw_rect(win_x + 10, win_y + 30, win_w - 100, 20, 0xFFFFFF); // 주소창
    draw_string("https://www.cloudos.com", win_x + 15, win_y + 35, 0x000000);
    draw_string("Go", win_x + win_w - 40, win_y + 35, 0xFFFFFF);
    draw_rect(win_x + win_w - 50, win_y + 30, 40, 20, 0x0A84FF); // Go 버튼

    // 2. 웹 페이지 콘텐츠 영역
    draw_rect(win_x, win_y + 55, win_w, win_h - 55, 0xFFFFFF); // 흰 배경
    draw_string("Welcome to CloudBrowser", win_x + 20, win_y + 75, 0x000000);
    draw_string("This is a fully native rendered web page.", win_x + 20, win_y + 95, 0x333333);
}
