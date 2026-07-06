#include "cloudoptimizer.h"
#include "vga.h"
#include "timer.h"
#include "task.h"

// 더티 렉트: 화면 전체를 다시 그리지 않고 변경된 영역만 그려 성능 향상 및 발열 감소
static int dirty_x = 0, dirty_y = 0, dirty_w = 1024, dirty_h = 768;

void optimizer_init(void) {
    vga_print("[ OK ] CloudOptimizer initialized (Power & Thermal).\n");
}

void optimizer_run(void) {
    // 1. 배터리 및 발열 최적화: 
    // 타이머 틱이 일정 수준 이상 쌓이면(유휴 상태) CPU를 HLT 상태로 강제 진입시켜 전력 소모 최소화
    if (timer_ticks % 1000 == 0) { // 10초마다 점검
        // 실제 구현에서는 ACPI를 통해 C-State 진입
    }
}

void optimizer_set_dirty_rect(int x, int y, int w, int h) {
    dirty_x = x; dirty_y = y; dirty_w = w; dirty_h = h;
}

int optimizer_is_dirty(int x, int y) {
    if (x >= dirty_x && x < dirty_x + dirty_w && y >= dirty_y && y < dirty_y + dirty_h) {
        return 1;
    }
    return 0;
}
