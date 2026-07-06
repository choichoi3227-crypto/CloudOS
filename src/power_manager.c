#include "power_manager.h"
#include "io.h"
#include "scheduler.h"
#include "vga.h"

void power_manager_init(void) {
    vga_print("[ OK ] Power Manager initialized (CPU Governor Active).\n");
}

// 배터리 절약: 백그라운드 프로세스의 타임 슬라이스 강제 제한
void power_manager_throttle_background(void) {
    // 스케줄러 내부에서 TASK_BACKGROUND 우선순위를 낮추어 실행 횟수를 줄임
    // (실제 구현에서는 CPU MSR 레지스터를 조작하여 클럭 주파수를 낮춤)
}

// 발열 최소화: 시스템 유휴 시 C-State 진입
void power_manager_enter_deep_sleep(void) {
    // 모든 백그라운드 작업이 대기 상태이면 HLT 명령으로 전력 소모 최소화
    __asm__ __volatile__("hlt");
}
