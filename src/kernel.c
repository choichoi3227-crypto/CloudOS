// kernel.c
#include "types.h"
#include "graphics.h"
#include "gpu.h"
#include "wm.h"
#include "vga.h"
#include "io.h"
#include "string.h"

// ==========================================
// 상용 OS 부트로더 통신 (Multiboot2 표준 파싱)
// ==========================================
typedef struct {
    uint32_t type;
    uint32_t size;
} multiboot_tag_t;

typedef struct {
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
} multiboot_fb_tag_t;

// 부트로더(boot.asm)에서 넘겨주는 프레임버퍼 전역 변수 (외부 참조)
// 만약 boot.asm에서 다른 방식으로 넘긴다면 이 부분을 boot.asm의 로직에 맞게 수정하세요.
extern uint32_t boot_fb_addr;
extern uint32_t boot_fb_pitch;
extern uint32_t boot_fb_width;
extern uint32_t boot_fb_height;

// ==========================================
// 커널 메인 함수 (OS의 시작점)
// ==========================================
void kernel_main(void) {
    // 1. VGA 텍스트 모드 초기화 (부팅 로그 출력을 위해)
    vga_init();

    vga_print("[CloudOS] Kernel Booting...\n");
    vga_print("[CloudOS] Initializing Hardware Interrupts...\n");

    // [중요] 기존 레포의 PIC, IDT, 키보드, 마우스 초기화 함수가 있다면 여기서 호출하세요.
    // 예: idt_init(); pic_init(); keyboard_init(); mouse_init();
    
    // ==========================================
    // 2. GPU 하드웨어 스캔 및 가속 준비 (실패해도 소프트웨어 렌더링으로 자동 폴백됨)
    // ==========================================
    vga_print("[CloudOS] Scanning GPU...\n");
    gpu_init();

    // ==========================================
    // 3. 그래픽 파이프라인 초기화 (더블버퍼링, 프론트버퍼 매핑)
    // ==========================================
    vga_print("[CloudOS] Initializing Graphics Pipeline...\n");
    
    // 부트로더가 제공한 프레임버퍼 정보를 그래픽 엔진에 전달
    uint32_t *fb_ptr = (uint32_t *)(uint64_t)boot_fb_addr;
    int fb_w = (int)boot_fb_width;
    int fb_h = (int)boot_fb_height;
    int fb_p = (int)boot_fb_pitch;

    if (fb_ptr == NULL || fb_w == 0 || fb_h == 0) {
        vga_print("[ERROR] Framebuffer not found! Halting.\n");
        for(;;) asm volatile("hlt");
    }

    graphics_init(fb_ptr, fb_w, fb_h, fb_p);
    vga_print("[CloudOS] Graphics Ready.\n");

    // [중요] 기존 레포의 파일시스템(CloudFS), 네트워크, ACPI 등 초기화가 있다면 여기서 호출하세요.
    // 예: cloudfs_init(); net_init();

    vga_print("[CloudOS] Starting Window Manager...\n");

    // ==========================================
    // 4. 윈도우 매니저 실행 (여기서 OS가 무한 루프를 돌며 GUI를 렌더링함)
    // ==========================================
    wm_run();
    
    // wm_run()은 반환하지 않으므로 아래 코드는 실행되지 않음
    for(;;) asm volatile("hlt");
}
