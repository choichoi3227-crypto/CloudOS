#include "ide.h"
#include "io.h"

#define ATA_DATA 0x1F0
#define ATA_ERROR 0x1F1
#define ATA_COUNT 0x1F2
#define ATA_LBA_LO 0x1F3
#define ATA_LBA_MID 0x1F4
#define ATA_LBA_HI 0x1F5
#define ATA_DRIVE 0x1F6
#define ATA_CMD 0x1F7
#define ATA_STATUS 0x1F7

int ide_read_sectors(uint64_t lba, uint32_t count, void* buf) {
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_COUNT, count);
    outb(ATA_LBA_LO, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_CMD, 0x20); // Read PIO

    uint16_t* ptr = (uint16_t*)buf;
    for (uint32_t i = 0; i < count; i++) {
        while (!(inb(ATA_STATUS) & 0x08));
        
        for (int j = 0; j < 256; j++) {
            ptr[j] = inw(ATA_DATA);
        }
        ptr += 256;
    }
    return count * 512;
}

int ide_write_sectors(uint64_t lba, uint32_t count, const void* buf) {
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_COUNT, count);
    outb(ATA_LBA_LO, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_CMD, 0x30); // Write PIO

    const uint16_t* ptr = (const uint16_t*)buf;
    for (uint32_t i = 0; i < count; i++) {
        while (!(inb(ATA_STATUS) & 0x08));
        
        for (int j = 0; j < 256; j++) {
            outw(ATA_DATA, ptr[j]);
        }
        ptr += 256;
    }
    return count * 512;
}
