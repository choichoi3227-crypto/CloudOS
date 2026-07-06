#include "mouse.h"
#include "io.h"

static int mouse_x = 512;
static int mouse_y = 384;
static int mouse_btn = 0;
static int mouse_wheel = 0;
static uint8_t mouse_cycle = 0;
static uint8_t mouse_buf[4]; // 휠 지원을 위해 4바이트로 확장

void mouse_init(void) {
    outb(0x64, 0xA8); 
    outb(0x64, 0x20); 
    uint8_t status = inb(0x60);
    status |= 0x02;   
    status &= ~0x20;
    outb(0x64, 0x60);
    outb(0x60, status);
    outb(0x64, 0xD4);
    outb(0x60, 0xF4); 
}

void mouse_handler(void) {
    uint8_t data = inb(0x60);
    
    switch (mouse_cycle) {
        case 0:
            mouse_buf[0] = data;
            // 패킷 시작 비트(3번 비트) 확인
            if (data & 0x08) mouse_cycle = 1;
            break;
        case 1:
            mouse_buf[1] = data;
            mouse_cycle = 2;
            break;
        case 2:
            mouse_buf[2] = data;
            mouse_cycle = 3; // 4번째 바이트(휠) 대기
            break;
        case 3:
            mouse_buf[3] = data;
            mouse_cycle = 0;
            
            mouse_btn = mouse_buf[0] & 0x03; // Left, Right Button
            
            // 이동 거리 계산
            int dx = mouse_buf[1];
            int dy = mouse_buf[2];
            
            // 마우스 가속도 적용 (노트북 터치패드 사용성 개선)
            if (dx < -10 || dx > 10) dx *= 2;
            if (dy < -10 || dy > 10) dy *= 2;

            if (mouse_buf[0] & 0x10) mouse_x -= (256 - dx);
            else mouse_x += dx;
            
            if (mouse_buf[0] & 0x20) mouse_y += (256 - dy);
            else mouse_y -= dy;
            
            // 휠 스크롤 (상하)
            // mouse_buf[3]의 부호에 따라 +1 또는 -1
            if (mouse_buf[3] != 0) {
                mouse_wheel = (mouse_buf[3] & 0x80) ? -1 : 1;
            } else {
                mouse_wheel = 0;
            }
            
            // 경계 제한
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= 1024) mouse_x = 1023;
            if (mouse_y >= 768) mouse_y = 767;
            break;
    }
}

int mouse_get_x(void) { return mouse_x; }
int mouse_get_y(void) { return mouse_y; }
int mouse_get_btn(void) { return mouse_btn; }
int mouse_get_wheel(void) { 
    int w = mouse_wheel; 
    mouse_wheel = 0; // 읽으면 초기화
    return w; 
}
