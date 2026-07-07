// keyboard.c
#include "keyboard.h"
#include "io.h"
#include "wm.h"
#include "vga.h"

#define KBD_PORT_DATA 0x60
#define KBD_PORT_CMD  0x64

static uint8_t keyboard_layout[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int shift_pressed = 0;
static int ext_key = 0;

void keyboard_init(void) {
    vga_print("[KBD] Keyboard Initialized.\n");
}

void keyboard_irq_handler(void) {
    uint8_t status = inb(KBD_PORT_CMD);
    if (status & 0x01) {
        uint8_t scancode = inb(KBD_PORT_DATA);

        // 확장 키 처리 (방향키, 엔터 등)
        if (scancode == 0xE0) {
            ext_key = 1;
            return;
        }

        // 키 떼기 처리
        if (scancode & 0x80) {
            scancode &= ~0x80;
            if (scancode == 0x2A || scancode == 0x36) shift_pressed = 0;
            ext_key = 0;
            return;
        }

        // Shift 상태 추적
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            return;
        }

        int key = 0;

        // 확장 키 (방향키 등) 처리
        if (ext_key) {
            if (scancode == 0x48) key = 0x01; // UP
            else if (scancode == 0x50) key = 0x02; // DOWN
            else if (scancode == 0x4B) key = 0x03; // LEFT
            else if (scancode == 0x4D) key = 0x04; // RIGHT
            ext_key = 0;
        } else {
            // 일반 키 변환
            char c = keyboard_layout[scancode];
            if (c >= 'a' && c <= 'z') {
                key = shift_pressed ? (c - 32) : c;
            } else if (c >= '1' && c <= '9') {
                key = shift_pressed ? "!@#$%^&*("[c - '1'] : c;
            } else if (c == '0') {
                key = shift_pressed ? ')' : '0';
            } else {
                key = c; // 엔터, 스페이스, 백스페이스 등
            }
        }

        // ==========================================
        // WM 이벤트 큐에 푸시
        // ==========================================
        if (key != 0) {
            wm_event_t ev = {0};
            ev.type = WM_EVENT_KEY_DOWN;
            ev.key = key;
            wm_push_input_event(&ev);
        }
    }
}
