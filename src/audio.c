#include "audio.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"

#define HDA_BASE 0x600000 // 오디오 컨트롤러 MMIO 매핑 지점

static volatile uint8_t* hda_mem;

// PCI 버스를 스캔하여 Intel HDA 오디오 컨트롤러 찾기
static uint32_t find_hda_controller() {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr);
            uint32_t id = inl(0xCFC);
            if (id == 0xFFFFFFFF) continue;

            outl(0xCF8, addr | 0x08);
            uint32_t class_code = inl(0xCFC) >> 16;
            
            // Class 0x04 (Multimedia), Subclass 0x01 (Audio), Interface 0x00 (HDA)
            if (class_code == 0x040100) {
                outl(0xCF8, addr | 0x24); // BAR0
                return inl(0xCFC) & 0xFFFFFFF0;
            }
        }
    }
    return 0;
}

void audio_init(void) {
    uint32_t phys_addr = find_hda_controller();
    if (!phys_addr) {
        vga_print("[WARN] Intel HDA Audio Controller not found.\n");
        return;
    }

    // HDA 메모리 영역 매핑
    vmm_map_page(phys_addr, HDA_BASE, 0x3); // Present | Writable
    hda_mem = (volatile uint8_t*)HDA_BASE;

    // 컨트롤러 리셋 (CRST 비트 0 설정 후 1로 설정)
    // 실제 구현에서는 CORB/RIRB 버퍼 설정 및 코덱 초기화가 필요함
    vga_print("[ OK ] Intel HDA Audio Controller found and mapped.\n");
}
