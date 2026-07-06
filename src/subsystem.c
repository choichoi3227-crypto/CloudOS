#include "subsystem.h"
#include "vga.h"
#include "elf.h"
#include "string.h"

void subsystem_init(void) {
    vga_print("[ OK ] Compatibility Subsystem initialized (Win/Linux).\n");
}

// Windows 앱(PE 포맷) 실행 로직 (Wine 스타일 스텁)
int exec_windows_app(const char* path) {
    vga_print("[Subsys] Loading Windows PE executable: ");
    vga_print(path);
    vga_print("\n");
    vga_print("[Subsys] Translating Win32 API to CloudOS API...\n");
    // 실제 구현에서는 PE 헤더 파싱, API 썽킹, 레지스트리 에뮬레이션 필요
    return 0;
}

// Linux 앱(ELF 포맷) 실행 로직 (WSL 스타일 스텁)
int exec_linux_app(const char* path) {
    vga_print("[Subsys] Loading Linux ELF executable: ");
    vga_print(path);
    vga_print("\n");
    // CloudOS의 ELF 로더를 호출하여 네이티브 실행
    return elf_load_and_exec(path);
}
