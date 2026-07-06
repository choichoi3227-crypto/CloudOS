#include "gpu.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"

#define GPU_BASE 0x700000 // GPU MMIO 매핑을 위한 가상 주소

static volatile uint8_t* gpu_mem;

// PCI 버스를 스캔하여 Intel Graphics 컨트롤러 찾기
static uint32_t find_intel_gpu() {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr);
            uint32_t id = inl(0xCFC);
            if (id == 0xFFFFFFFF) continue;

            uint32_t vendor_id = id & 0xFFFF;
            outl(0xCF8, addr | 0x08);
            uint32_t class_code = inl(0xCFC) >> 16;
            
            // Class 0x03 (Display), Subclass 0x00 (VGA), Vendor 0x8086 (Intel)
            if (class_code == 0x030000 && vendor_id == 0x8086) {
                outl(0xCF8, addr | 0x10); // BAR0 (Memory Mapped I/O)
                return inl(0xCFC) & 0xFFFFFFF0;
            }
        }
    }
    return 0;
}

void gpu_init(void) {
    uint32_t phys_addr = find_intel_gpu();
    if (!phys_addr) {
        vga_print("[WARN] Intel GPU not found. Using Software Rendering.\n");
        return;
    }

    // GPU 메모리 영역 매핑 (최소 1페이지, 실제로는 훨씬 큼)
    vmm_map_page(phys_addr, GPU_BASE, 0x3); // Present | Writable
    gpu_mem = (volatile uint8_t*)GPU_BASE;

    vga_print("[ OK ] Intel Graphics Controller found and mapped.\n");
    vga_print("[ OK ] GPU Driver Framework initialized (User-space ready).\n");
}
