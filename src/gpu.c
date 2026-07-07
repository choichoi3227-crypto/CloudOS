// gpu.c
#include "gpu.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"

#define GPU_BASE 0x700000
static volatile uint8_t *gpu_mem;
static int gpu_available = 0;

static uint32_t find_intel_gpu(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1U << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr); uint32_t id = inl(0xCFC);
            if (id == 0xFFFFFFFF) continue;
            outl(0xCF8, addr | 0x08); uint32_t class_code = inl(0xCFC) >> 16;
            if (class_code == 0x030000 && (id & 0xFFFF) == 0x8086) {
                outl(0xCF8, addr | 0x10); return inl(0xCFC) & 0xFFFFFFF0;
            }
        }
    }
    return 0;
}

void gpu_init(void) {
    uint32_t phys = find_intel_gpu();
    if (!phys) { vga_print("[WARN] GPU not found. Software FB.\n"); return; }
    vmm_map_page(phys, GPU_BASE, 0x3);
    gpu_mem = (volatile uint8_t *)GPU_BASE;
    gpu_available = 1;
}

int gpu_is_available(void) { return gpu_available; }
void gpu_blit_to_framebuffer(const void *src, size_t size) { (void)src; (void)size; }
void gpu_fill_rect(int x, int y, int w, int h, uint32_t color) { (void)x; (void)y; (void)w; (void)h; (void)color; }
