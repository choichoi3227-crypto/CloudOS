#include "include/types.h"

// VGA 텍스트 모드 메모리 주소
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static int cursor_x = 0;
static int cursor_y = 0;

// 화면 출력 함수
void kprint(const char* str) {
    uint16_t* vga = (uint16_t*)VGA_MEMORY;
    while (*str) {
        if (*str == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            vga[cursor_y * VGA_WIDTH + cursor_x] = (uint16_t)*str | 0x0F << 8;
            cursor_x++;
            if (cursor_x >= VGA_WIDTH) { cursor_x = 0; cursor_y++; }
        }
        str++;
    }
}

// 문자열 비교 (로그인 검증용)
int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

// 간단한 키보드 입력 대기 (루프)
char get_key() {
    // 키보드 컨트롤러(포트 0x60)에서 데이터 읽기 대기 (Stub)
    // 실제 구현에서는 인터럽트(IDT)를 통해 처리합니다.
    char c = 0;
    // 여기서는 시뮬레이션을 위해 'a' 반환
    return 'a'; 
}

// 로그인 화면
int login_screen() {
    kprint("=== CloudOS Login ===\n");
    kprint("User: admin\n");
    kprint("Pass: ");
    
    char pass[16];
    int idx = 0;
    while (idx < 15) {
        char c = get_key();
        if (c == '\n') break;
        pass[idx++] = c;
    }
    pass[idx] = '\0';
    
    // 실제로는 로컬 DB와 USB를 조회해야 하지만, 커널 레벨에서는 하드코딩
    if (strcmp(pass, "password") == 0) {
        kprint("\nLogin Success!\n");
        return 1;
    }
    kprint("\nLogin Failed!\n");
    return 0;
}

// 메인 커널
void kernel_main() {
    // 화면 초기화
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        ((uint16_t*)VGA_MEMORY)[i] = 0;
    }

    kprint("CloudOS Kernel v1.0 (Native Boot)\n");
    kprint("Initializing Hardware...\n");
    
    // 로그인 루프
    while (!login_screen()) {
        // 실패 시 계속 루프
    }
    
    // 로그인 성공 후 데스크탑 진입 (실제로는 GUI 모드 전환)
    kprint("Loading Desktop Environment (50 Apps)...\n");
    
    while (1) {
        // 메인 시스템 루프
        __asm__ __volatile__("hlt");
    }
}
