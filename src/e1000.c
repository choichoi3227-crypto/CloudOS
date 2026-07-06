#include "e1000.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"
#include "string.h"

#define E1000_IO_BASE 0xC000
#define E1000_IMS 0x00D0
#define E1000_ICR 0x00C0

static volatile uint8_t* e1000_mem;

void e1000_init(void) {
    e1000_mem = (uint8_t*)E1000_IO_BASE;
    vmm_map_page(E1000_IO_BASE, (virtaddr_t)e1000_mem, 0x3); // RW
    
    // 인터럽트 마스크 설정 (Receive Timer)
    *(uint32_t*)(e1000_mem + E1000_IMS) = (1 << 7);
    
    vga_print("[ OK ] E1000 Network Card initialized.\n");
}

void e1000_handler(void) {
    uint32_t status = *(uint32_t*)(e1000_mem + E1000_ICR);
    if (status) {
        vga_print("[E1000] Network interrupt received!\n");
        // 실제 구현에서는 RX 링 버퍼에서 패킷을 읽어 net_handle_packet() 호출
        // 여기서는 수신 확인만 로그로 출력
    }
    // EOI는 idt.c에서 전송
}

void e1000_send_packet(void* data, uint32_t len) {
    vga_print("[E1000] Sending packet...\n");
    // 실제 구현에서는 TX 링 버퍼에 데이터를 쓰고 전송 명령 내림
}
