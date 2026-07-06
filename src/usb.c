#include "usb.h"
#include "io.h"
#include "vmm.h"
#include "vga.h"

#define USB_XHCI_BASE 0x500000 // 가상 메모리 매핑 지점

static volatile uint8_t* xhci_mem;

// PCI 버스를 스캔하여 xHCI (USB 3.0) 컨트롤러 찾기
static uint32_t find_xhci_controller() {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11);
            outl(0xCF8, addr);
            uint32_t id = inl(0xCFC);
            if (id == 0xFFFFFFFF) continue;

            outl(0xCF8, addr | 0x08);
            uint32_t class_code = inl(0xCFC) >> 16;
            
            // Class 0x0C (Serial Bus), Subclass 0x03 (USB), Interface 0x30 (xHCI)
            if (class_code == 0x0C0330) {
                outl(0xCF8, addr | 0x24); // BAR0
                return inl(0xCFC) & 0xFFFFFFF0;
            }
        }
    }
    return 0;
}

void usb_init(void) {
    uint32_t phys_addr = find_xhci_controller();
    if (!phys_addr) {
        vga_print("[WARN] xHCI USB Controller not found. Falling back to PS/2.\n");
        return;
    }

    // xHCI 메모리 영역 매핑
    vmm_map_page(phys_addr, USB_XHCI_BASE, 0x3); // Present | Writable
    xhci_mem = (volatile uint8_t*)USB_XHCI_BASE;

    vga_print("[ OK ] xHCI USB Controller found and mapped.\n");
    // 실제 구현에서는 xHCI 레지스터를 조작하여 USB 장치(마우스, 키보드) 초기화
}
