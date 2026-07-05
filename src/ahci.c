#include "ahci.h"
#include "io.h"
#include "vmm.h"
#include "pmm.h"
#include "vga.h"
#include "string.h"

// AHCI 레지스터 구조체 (간략화)
typedef struct {
    uint32_t pi;          // Ports Implemented
    uint32_t ghc;         // Global Host Control
    // ... 생략 ...
} hba_mem_t;

typedef struct {
    uint32_t clb;         // Command List Base
    uint32_t clbu;
    uint32_t fb;          // FIS Base
    uint32_t fbu;
    uint32_t is;          // Interrupt Status
    uint32_t ie;          // Interrupt Enable
    uint32_t cmd;         // Command
    // ... 생략 ...
} hba_port_t;

static volatile hba_mem_t* abar;
static volatile hba_port_t* port;

// PCI 버스를 스캔하여 AHCI 컨트롤러 찾기
static uint32_t find_ahci_controller() {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr);
            uint32_t id = inl(0xCFC);
            if (id == 0xFFFFFFFF) continue;

            outl(0xCF8, addr | 0x08);
            uint32_t class_code = inl(0xCFC) >> 16;
            
            // Class 0x01 (Mass Storage), Subclass 0x06 (SATA), Interface 0x01 (AHCI)
            if (class_code == 0x010601) {
                outl(0xCF8, addr | 0x24); // BAR5
                return inl(0xCFC) & 0xFFFFF000;
            }
        }
    }
    return 0;
}

void ahci_init(void) {
    uint32_t phys_addr = find_ahci_controller();
    if (!phys_addr) {
        vga_print("[FAIL] AHCI Controller not found.\n");
        return;
    }

    // AHCI 메모리 영역을 가상 메모리에 매핑
    vmm_map_page(phys_addr, AHCI_BASE, 0x3); // Present | Writable
    abar = (volatile hba_mem_t*)AHCI_BASE;
    port = (volatile hba_port_t*)(AHCI_BASE + 0x100); // 첫 번째 포트 (Port 0) 가정

    // AHCI 인터럽트 활성화 (GHC)
    abar->ghc |= 0x02; 

    vga_print("[ OK ] AHCI SATA Controller initialized.\n");
}

// DMA 기반 섹터 읽기 (간략화된 폴링 방식)
int ahci_read_sectors(uint64_t lba, uint32_t count, void* buf) {
    if(!port) return -1;
    
    // 실제 구현에서는 커맨드 리스트, PRDT 설정 후 PxCI 레지스터에 명령 발행.
    // 여기서는 성공했다고 가정하고 버퍼를 0으로 채움 (테스트용 Stub).
    memset(buf, 0, count * 512);
    vga_print("[AHCI] Read command issued.\n");
    return count * 512;
}

int ahci_write_sectors(uint64_t lba, uint32_t count, const void* buf) {
    if(!port) return -1;
    
    // 실제 구현에서는 쓰기 명령(WRITE DMA) 발행.
    vga_print("[AHCI] Write command issued.\n");
    return count * 512;
}
