#include "perfect_os.h"
#include "vga.h"
#include "io.h"

void power_acpi_init(void) {
    vga_print("[Power] Initializing ACPI 5.0 (Modern Standby)...\n");
    // FADT 테이블 파싱하여 PM1a_CNT_BLK 찾기
    // CPU P-State (DVFS) 및 C-State 설정
}

void power_enter_modern_standby(void) {
    // 윈도우의 Modern Standby처럼 백그라운드를 완전히 멈추고 극저전력 상태 진입
    // 인터럽트가 들어오면 즉시 깨어남 (S0ix)
    __asm__ __volatile__("hlt");
}
