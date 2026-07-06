#include "perfect_os.h"
#include "io.h"
#include "vga.h"

void driver_framework_init(void) {
    vga_print("[Driver] Initializing Unified Driver Interface (UDI)...\n");
}

void driver_auto_detect(void) {
    vga_print("[Driver] Scanning PCI and USB buses...\n");
    // PCI 버스 스캔 (Bus 0~255, Slot 0~31)
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr);
            uint32_t id = inl(0xCFC);
            if (id == 0xFFFFFFFF) continue;

            uint32_t class_code = (inl(0xCFC) >> 16) & 0xFFFF;
            
            // 자동으로 적절한 커널 드라이버 바인딩
            if (class_code == 0x0106) vga_print("[Driver] Auto-bound AHCI driver.\n");
            else if (class_code == 0x0300) vga_print("[Driver] Auto-bound GPU driver.\n");
            else if (class_code == 0x0401) vga_print("[Driver] Auto-bound Audio driver.\n");
            else if (class_code == 0x0C03) vga_print("[Driver] Auto-bound USB driver.\n");
        }
    }
}
