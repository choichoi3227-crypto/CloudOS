// src/mouse.c (기존 파일 덮어쓰기)
#include "../include/types.h" // [추가됨] int8_t, int16_t 등을 사용하기 위함
#include "mouse.h"
#include "io.h"
#include "wm.h"
#include "vga.h"

#define MOUSE_PORT_CMD  0x64
#define MOUSE_PORT_DATA 0x60

static int mouse_x = 0;
static int mouse_y = 0;
static uint8_t mouse_cycle = 0;
static int8_t  mouse_byte[3]; // [수정] int8_t 사용
static uint8_t mouse_buttons = 0;

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) { if ((inb(MOUSE_PORT_CMD) & 1) == 1) return; }
    } else {
        while (timeout--) { if ((inb(MOUSE_PORT_CMD) & 2) == 0) return; }
    }
}

static void mouse_write(uint8_t value) {
    mouse_wait(1);
    outb(MOUSE_PORT_CMD, 0xD4);
    mouse_wait(1);
    outb(MOUSE_PORT_DATA, value);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_PORT_DATA);
}

void mouse_init(void) {
    vga_print("[Mouse] Initializing PS/2 Mouse...\n");
    mouse_wait(1); outb(MOUSE_PORT_CMD, 0xA8);
    mouse_wait(1); outb(MOUSE_PORT_CMD, 0x20); uint8_t status = mouse_read();
    status |= 0x02; status &= ~0x20;
    mouse_wait(1); outb(MOUSE_PORT_CMD, 0x60); mouse_wait(1); outb(MOUSE_PORT_DATA, status);
    mouse_write(0xF6); mouse_read();
    mouse_write(0xF4); mouse_read();
    vga_print("[Mouse] PS/2 Mouse Enabled.\n");
}

void mouse_irq_handler(void) {
    uint8_t status = inb(MOUSE_PORT_CMD);
    if (!(status & 0x20)) return;

    uint8_t data = inb(MOUSE_PORT_DATA);
    mouse_byte[mouse_cycle++] = data;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;
        uint8_t new_buttons = mouse_byte[0] & 0x07;
        
        // [수정] 부호 있는 16비트 정수로 캐스팅 (Sign Extension 처리)
        int16_t dx = (int16_t)mouse_byte[1];
        int16_t dy = (int16_t)mouse_byte[2];
        
        if (mouse_byte[0] & 0x10) dx |= 0xFF00;
        if (mouse_byte[0] & 0x20) dy |= 0xFF00;

        mouse_x += dx;
        mouse_y += dy;

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1023) mouse_x = 1023;
        if (mouse_y > 767) mouse_y = 767;

        wm_event_t move_ev = {0};
        move_ev.type = WM_EVENT_MOUSE_MOVE;
        move_ev.x = mouse_x;
        move_ev.y = mouse_y;
        move_ev.dx = dx;
        move_ev.dy = dy;
        wm_push_input_event(&move_ev);

        if (new_buttons != mouse_buttons) {
            if ((new_buttons & 0x01) && !(mouse_buttons & 0x01)) {
                wm_event_t down_ev = {0};
                down_ev.type = WM_EVENT_MOUSE_DOWN;
                down_ev.button = 1;
                down_ev.x = mouse_x;
                down_ev.y = mouse_y;
                wm_push_input_event(&down_ev);
            } else if (!(new_buttons & 0x01) && (mouse_buttons & 0x01)) {
                wm_event_t up_ev = {0};
                up_ev.type = WM_EVENT_MOUSE_UP;
                up_ev.button = 1;
                up_ev.x = mouse_x;
                up_ev.y = mouse_y;
                wm_push_input_event(&up_ev);
            }
        }
        mouse_buttons = new_buttons;
    }
}
