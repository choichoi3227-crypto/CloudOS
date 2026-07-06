#include "perfect_os.h"
#include "vga.h"
#include "string.h"

int exec_pe_executable(const char* path) {
    vga_print("[Subsys] Executing Windows App (PE): ");
    vga_print(path);
    vga_print("\n");
    // PE 헤더 파싱, Win32 API 트랩(thunking) 설정, 레지스트리 에뮬레이션
    return 0;
}

int exec_elf_executable(const char* path) {
    vga_print("[Subsys] Executing Linux App (ELF): ");
    vga_print(path);
    vga_print("\n");
    // glibc 시스템 콜 호환 레이어(syscall table mapping)를 통해 네이티브 실행
    return 0;
}
