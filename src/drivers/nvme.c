#include "nvme.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"

#define NVME_BASE 0x800000

static volatile uint8_t* nvme_mem;

void nvme_init(void) {
    // PCI 스캔 (Class 0x01, Subclass 0x08)
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr);
            if (inl(0xCFC) == 0xFFFFFFFF) continue;

            outl(0xCF8, addr | 0x08);
            uint32_t class_code = inl(0xCFC) >> 16;
            if (class_code == 0x0108) { // NVMe
                outl(0xCF8, addr | 0x10); // BAR0
                uint32_t phys = inl(0xCFC) & 0xFFFFFFF0;
                vmm_map_page(phys, NVME_BASE, 0x3);
                nvme_mem = (volatile uint8_t*)NVME_BASE;
                vga_print("[ OK ] NVMe Controller initialized.\n");
                return;
            }
        }
    }
}
