#include "mouse.h"
#include "io.h"

static int mouse_x = 512;
static int mouse_y = 384;
static int mouse_btn = 0;
static uint8_t mouse_cycle = 0;
static uint8_t mouse_buf[3];

void mouse_init(void) {
    outb(0x64, 0xA8); // 보조 장치 활성화
    outb(0x64, 0x20); // 상태 읽기
    uint8_t status = inb(0x60);
    status |= 0x02;   // IRQ12 활성화
    status &= ~0x20;
    outb(0x64, 0x60);
    outb(0x60, status);
    outb(0x64, 0xD4);
    outb(0x60, 0xF4); // 데이터 리포팅 활성화
}

void mouse_handler(void) {
    uint8_t data = inb(0x60);
    
    switch (mouse_cycle) {
        case 0:
            mouse_buf[0] = data;
            mouse_cycle = 1;
            break;
        case 1:
            mouse_buf[1] = data;
            mouse_cycle = 2;
            break;
        case 2:
            mouse_buf[2] = data;
            mouse_cycle = 0;
            
            mouse_btn = mouse_buf[0] & 0x01; // 왼쪽 버튼 상태
            
            // X 이동
            if (mouse_buf[0] & 0x10) mouse_x -= (256 - mouse_buf[1]);
            else mouse_x += mouse_buf[1];
            
            // Y 이동 (반전)
            if (mouse_buf[0] & 0x20) mouse_y += (256 - mouse_buf[2]);
            else mouse_y -= mouse_buf[2];
            
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
