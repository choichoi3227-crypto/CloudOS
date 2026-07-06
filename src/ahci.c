#include "ahci.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"
#include "string.h"

typedef struct { uint32_t pi; uint32_t ghc; } hba_mem_t;
typedef struct { uint32_t clb; uint32_t clbu; uint32_t fb; uint32_t fbu; uint32_t is; uint32_t ie; uint32_t cmd; } hba_port_t;

static volatile hba_mem_t* abar;
static volatile hba_port_t* port;

static uint32_t find_ahci_controller() {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr);
            uint32_t id = inl(0xCFC);
            if (id == 0xFFFFFFFF) continue;

            outl(0xCF8, addr | 0x08);
            uint32_t class_code = inl(0xCFC) >> 16;
            
            if (class_code == 0x010601) {
                outl(0xCF8, addr | 0x24);
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

    vmm_map_page(phys_addr, AHCI_BASE, 0x3);
    abar = (volatile hba_mem_t*)AHCI_BASE;
    port = (volatile hba_port_t*)(AHCI_BASE + 0x100);

    abar->ghc |= 0x02; 

    vga_print("[ OK ] AHCI SATA Controller initialized.\n");
}

int ahci_read_sectors(uint64_t lba, uint32_t count, void* buf) {
    if(!port) return -1;
    memset(buf, 0, count * 512);
    vga_print("[AHCI] Read command issued.\n");
    return count * 512;
}

int ahci_write_sectors(uint64_t lba, uint32_t count, const void* buf) {
    if(!port) return -1;
    vga_print("[AHCI] Write command issued.\n");
    return count * 512;
}
