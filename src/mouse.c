// mouse.c
#include "mouse.h"
#include "io.h"
#include "wm.h"
#include "vga.h"

// 마우스 하드웨어 포트
#define MOUSE_PORT_CMD  0x64
#define MOUSE_PORT_DATA 0x60

// 마우스 상태 저장
static int mouse_x = 0;
static int mouse_y = 0;
static uint8_t mouse_cycle = 0;
static int8_t  mouse_byte[3];
static uint8_t mouse_buttons = 0;

// PS/2 컨트롤러에 명령 보내기 (대기 로직 포함)
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

// 하드웨어 초기화
void mouse_init(void) {
    vga_print("[Mouse] Initializing PS/2 Mouse...\n");
    
    // 마우스 활성화 명령 전송
    mouse_wait(1);
    outb(MOUSE_PORT_CMD, 0xA8); // IRQ12 활성화
    mouse_wait(1);
    outb(MOUSE_PORT_CMD, 0x20); // 컨트롤러 레지스터 읽기
    uint8_t status = mouse_read();
    status |= 0x02; // 컨트롤러가 마우스 데이터 받음 허용
    status &= ~0x20; // 마우스 클럭 활성화
    mouse_wait(1);
    outb(MOUSE_PORT_CMD, 0x60); // 컨트롤러 레지스터 쓰기
    mouse_wait(1);
    outb(MOUSE_PORT_DATA, status);

    // 마우스 디바이스에 기본 설정 전송
    mouse_write(0xF6); // 기본 파라미터 설정
    mouse_read();
    mouse_write(0xF4); // 스트리밍 모드 활성화 (데이터 전송 시작)
    mouse_read();
    
    vga_print("[Mouse] PS/2 Mouse Enabled.\n");
}

// 상용 OS 마우스 드라이버 핵심: 인터럽트 기반 패킷 파싱
void mouse_irq_handler(void) {
    uint8_t status = inb(MOUSE_PORT_CMD);
    if (!(status & 0x20)) return; // 마우스 데이터가 아니면 무시

    uint8_t data = inb(MOUSE_PORT_DATA);
    mouse_byte[mouse_cycle++] = data;

    // 3바이트 패킷이 완성되면 WM으로 이벤트 전송
    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        // 버튼 상태 업데이트
        uint8_t new_buttons = mouse_byte[0] & 0x07;
        
        // X, Y 이동량 계산 (12비트 부호 있는 정수 처리 - 상용 OS 필수 로직)
        int16_t dx = mouse_byte[1];
        int16_t dy = mouse_byte[2];
        
        // 부호 비트(Bit 4)가 1이면 음수이므로 상위 4비트를 모두 1로 채움 (Sign Extension)
        if (mouse_byte[0] & 0x10) dx |= 0xFF00;
        if (mouse_byte[0] & 0x20) dy |= 0xFF00;

        // 절대 좌표에 누적
        mouse_x += dx;
        mouse_y += dy;

        // 화면 밖으로 나가지 않게 제한
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > 1023) mouse_x = 1023;
        if (mouse_y > 767) mouse_y = 767;

        // ==========================================
        // WM 이벤트 큐에 비동기 푸시 (Lock-Free)
        // ==========================================
        
        // 1. 마우스 이동 이벤트 (항상 발생)
        wm_event_t move_ev = {0};
        move_ev.type = WM_EVENT_MOUSE_MOVE;
        move_ev.x = mouse_x;
        move_ev.y = mouse_y;
        move_ev.dx = dx;
        move_ev.dy = dy;
        wm_push_input_event(&move_ev);

        // 2. 버튼 상태 변화 감지 (버튼 눌림/떼기 이벤트)
        if (new_buttons != mouse_buttons) {
            // 왼쪽 버튼 (Bit 0)
            if ((new_buttons & 0x01) && !(mouse_buttons & 0x01)) {
                wm_event_t down_ev = {0};
                down_ev.type = WM_EVENT_MOUSE_DOWN;
                down_ev.button = 1; // 1 = Left
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
            // 오른쪽 버튼 (Bit 1) - 필요시 로직 추가 가능
            // 가운데 버튼 (Bit 2) - 필요시 로직 추가 가능
        }
        mouse_buttons = new_buttons;
    }
}
