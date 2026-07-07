// gpu.c
#include "gpu.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"

#define GPU_BASE 0x700000 // GPU MMIO 매핑을 위한 가상 주소
static volatile uint8_t *gpu_mem;
static int gpu_available = 0;

// PCI 버스를 스캔하여 Intel Graphics 컨트롤러 찾기
static uint32_t find_intel_gpu(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1U << 31) | (bus << 16) | (slot << 11);
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
        gpu_available = 0;
        return;
    }
    // GPU 메모리 영역 매핑 (최소 1페이지, 실제로는 훨씬 큼)
    vmm_map_page(phys_addr, GPU_BASE, 0x3); // Present | Writable
    gpu_mem = (volatile uint8_t *)GPU_BASE;
    gpu_available = 1;
    vga_print("[ OK ] Intel Graphics Controller found and mapped.\n");
    vga_print("[ OK ] GPU Driver Framework initialized (User-space ready).\n");
}

int gpu_is_available(void) {
    return gpu_available;
}

// 백버퍼→프론트버퍼 블릿 (스텁)
void gpu_blit_to_framebuffer(const void *src, size_t size) {
    if (!gpu_available || !gpu_mem || !src) return;
    // TODO: 실제 Intel BLT 엔진 명령 스트림을 구성해서 하드웨어 가속 수행
    // 현재는 안전하게 소프트웨어 폴백을 사용하므로 no-op
    (void)size;
}

// 단순 사각형 채우기 (GPU 2D Fill 스텁)
void gpu_fill_rect(int x, int y, int w, int h, uint32_t color) {
    if (!gpu_available || !gpu_mem) return;
    // TODO: BLT 엔진의 COLOR_BLT 또는 PAT_BLT 사용하여 빠르게 채우기
    (void)x; (void)y; (void)w; (void)h; (void)color;
}
