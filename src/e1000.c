#include "e1000.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"

#define E1000_IO_BASE 0xC000
#define E1000_IMS 0x00D0
#define E1000_ICR 0x00C0

static volatile uint8_t* e1000_mem;

void e1000_init(void) {
    e1000_mem = (uint8_t*)E1000_IO_BASE;
    vmm_map_page(E1000_IO_BASE, (virtaddr_t)e1000_mem, 0x3);
    *(uint32_t*)(e1000_mem + E1000_IMS) = (1 << 7);
    vga_print("[ OK ] E1000 Network Card initialized.\n");
}

void e1000_handler(void) {
    uint32_t status = *(uint32_t*)(e1000_mem + E1000_ICR);
    if (status) {
        vga_print("[E1000] Network interrupt received!\n");
    }
}

void e1000_send_packet(void* data, uint32_t len) {
    vga_print("[E1000] Sending packet...\n");
}
