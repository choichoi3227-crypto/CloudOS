#include "acpi.h"
#include "vga.h"
#include "string.h"
#include "io.h"

// ACPI RSDP 구조체
typedef struct {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed)) rsdp_t;

// ACPI FADT 구조체 (간략화)
typedef struct {
    char signature[4];
    uint32_t length;
    // ... 생략 ...
    uint32_t pm1a_cnt_blk;
    // ... 생략 ...
} __attribute__((packed)) fadt_t;

static rsdp_t* rsdp = NULL;

// BIOS 영역에서 RSDP 테이블 탐색
static rsdp_t* find_rsdp() {
    // EBDA(Extended BIOS Data Area) 주소 읽기
    uint32_t ebda = *(uint16_t*)0x40E << 4;
    
    // EBDA 영역 스캔 (1KB)
    for (uint32_t i = ebda; i < ebda + 1024; i += 16) {
        if (memcmp((void*)i, "RSD PTR ", 8) == 0) {
            return (rsdp_t*)i;
        }
    }
    
    // 기본 BIOS 영역 스캔 (0xE0000 ~ 0xFFFFF)
    for (uint32_t i = 0xE0000; i < 0xFFFFF; i += 16) {
        if (memcmp((void*)i, "RSD PTR ", 8) == 0) {
            return (rsdp_t*)i;
        }
    }
    return NULL;
}

void acpi_init(void) {
    rsdp = find_rsdp();
    if (rsdp) {
        vga_print("[ OK ] ACPI Table found. Power management enabled.\n");
    } else {
        vga_print("[WARN] ACPI Table not found.\n");
    }
}

// 시스템 전원 차단 (QEMU/Bochs 작동 방식)
void acpi_power_off(void) {
    vga_print("System shutting down...\n");
    
    // QEMU/Bochs 에뮬레이터 종료 명령
    outw(0x604, 0x2000);
    
    // ACPI 방식 (PM1a_CNT_BLK의 SLP_TYP 설정 - 하드웨어 의존적)
    if (rsdp) {
        // 실제 하드웨어에서는 FADT를 파싱하여 pm1a_cnt_blk를 찾아야 함
        // outw(pm1a_cnt_blk, slp_type << 10 | 0x2000);
    }
    
    // 대기
    while(1) { __asm__ __volatile__("hlt"); }
}
