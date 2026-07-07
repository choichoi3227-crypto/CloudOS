// kernel.c
#include "types.h"
#include "graphics.h"
#include "gpu.h"
#include "wm.h"
#include "vga.h"
#include "idt.h"
#include "mouse.h"
#include "keyboard.h"

// ==========================================
// 부트로더(boot.asm)와의 약속 (Multiboad2 또는 커스텀 부트로더)
// boot.asm에서 메모리 맵이나 구조체로 넘겨준 값을 여기서 받음
// ==========================================
extern uint32_t boot_fb_addr;
extern uint32_t boot_fb_pitch;
extern uint32_t boot_fb_width;
extern uint32_t boot_fb_height;

void kernel_main(void) {
    // 1. 화면 초기화 (검은 바탕에 흰 글자로 로그 출력)
    vga_init();
    vga_print("========================================\n");
    vga_print("         CloudOS Kernel Booting         \n");
    vga_print("========================================\n");

    // ==========================================
    // 2. 하드웨어 인터럽트 시스템 기동 (가장 중요)
    // 이걸 안 하면 마우스/키보드를 꽂아도 OS가 반응하지 않음
    // ==========================================
    idt_init();       // PIC 리매핑 및 IDT 등록
    
    // 3. 입출력 장치 드라이버 초기화
    keyboard_init();  // 키보드 스캔코드 해석기 준비
    mouse_init();     // PS/2 마우스 하드웨어 활성화 (이제부터 마우스 움직임 감지 시작됨)

    // ==========================================
    // 4. 그래픽 파이프라인 초기화
    // ==========================================
    vga_print("[CloudOS] Initializing Graphics...\n");
    
    // 부트로더가 화면 메모리(프레임버퍼) 주소를 안 넘겨줬다면 강제 종료
    if (boot_fb_addr == 0) {
        vga_print("[FATAL ERROR] Bootloader did not provide Framebuffer!\n");
        for(;;) asm volatile("hlt");
    }

    // GPU 가속 칩셋 스캔 (없으면 자동으로 소프트웨어 렌더링 모드로 전환됨)
    gpu_init();

    // 프론트버퍼 메모리를 그래픽 엔진에 연결
    graphics_init(
        (uint32_t *)(uint64_t)boot_fb_addr, 
        (int)boot_fb_width, 
        (int)boot_fb_height, 
        (int)boot_fb_pitch
    );

    vga_print("[CloudOS] Graphics Subsystem Ready.\n");

    // ==========================================
    // 5. 윈도우 매니저 실행 (여기서 OS가 사용자의 입력을 기다리며 무한 루프)
    // ==========================================
    vga_print("[CloudOS] Starting Desktop Environment...\n");
    
    // wm_run() 함수 안에서 asm volatile("hlt"); 를 통해 전력을 절약하며 대기함
    // 마우스를 움직이거나 키를 누르면 하드웨어 인터럽트가 발생해 HLT가 풀리고 GUI가 갱신됨
    wm_run();
    
    // wm_run()은 정상적인 OS라면 절대 반환하지 않음
}
