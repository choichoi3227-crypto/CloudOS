// src/sys/bitlocker.h
#ifndef BITLOCKER_H
#define BITLOCKER_H

#include "../include/types.h"

#define CLOUDOS_FS_MAGIC 0xCAFEBABE
#define SECTOR_SIZE 512

// 볼륨 헤더 구조체 (디스크 첫 섹터에 저장됨)
typedef struct {
    uint32_t magic;
    uint8_t  is_encrypted;
    uint8_t  encryption_key[32];
    uint32_t checksum;
} cloudos_volume_header_t;

void bitlocker_init_volume(uint8_t* disk_ptr);
void bitlocker_encrypt_sector(uint8_t* sector_data, uint8_t* key);
void bitlocker_decrypt_sector(uint8_t* sector_data, uint8_t* key);

#endif
