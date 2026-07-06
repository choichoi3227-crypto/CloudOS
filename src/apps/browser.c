#include "browser.h"
#include "graphics.h"
#include "string.h"

void browser_render_page(int win_x, int win_y, const char* html) {
    int cx = win_x + 10, cy = win_y + 40;
    int in_tag = 0;
    
    // 간단한 HTML 파서 및 렌더링
    while (*html) {
        if (*html == '<') in_tag = 1;
        else if (*html == '>') in_tag = 0;
        else if (!in_tag) {
            if (*html == '\n') { cx = win_x + 10; cy += 16; }
            else { draw_char(*html, cx, cy, 0x000000); cx += 8; }
        }
        html++;
    }
}
