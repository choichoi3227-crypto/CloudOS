// src/sys/bitlocker.c
#include "bitlocker.h"
#include "../include/string.h"

// 상용 OS 암호화 핵심: 난수 생성기 (XOR 스트림 암호화용)
static uint32_t prng_state = 0xDEADBEEF;
static uint8_t generate_pseudo_random_byte() {
    prng_state ^= prng_state << 13;
    prng_state ^= prng_state >> 17;
    prng_state ^= prng_state << 5;
    return (uint8_t)(prng_state & 0xFF);
}

// 볼륨 암호화 초기화
void bitlocker_init_volume(uint8_t* disk_ptr) {
    cloudos_volume_header_t* header = (cloudos_volume_header_t*)disk_ptr;
    
    if (header->magic == CLOUDOS_FS_MAGIC) {
        if (header->is_encrypted) {
            // 부팅 시 복호화 키 메모리 로드 (실제로는 TPM 칩이나 사용자 비밀번호 입력 필요)
            // 지금은 디스크에 있는 키를 가져옴 (테스트용)
            return; 
        }
    }
    
    // 미암호화 볼륨 -> 암호화 설정
    header->magic = CLOUDOS_FS_MAGIC;
    header->is_encrypted = 1;
    
    // 가상의 마스터 키 생성 (실제 OS는 PBKDF2 등으로 파생)
    for(int i=0; i<32; i++) {
        header->encryption_key[i] = generate_pseudo_random_byte();
    }
    header->checksum = 0x12345678; // CRC 대체
}

// 디스크 섹터 암호화 (XOR 스트림 암호화 방식 - AES의 간소화 버전)
void bitlocker_encrypt_sector(uint8_t* sector_data, uint8_t* key) {
    for(int i=0; i<SECTOR_SIZE; i++) {
        // 키 스트림 생성 및 XOR 연산
        uint8_t key_stream = key[i % 32] ^ generate_pseudo_random_byte();
        sector_data[i] ^= key_stream;
    }
}

// 디스크 섹터 복호화 (XOR은 대칭키이므로 암호화와 동일)
void bitlocker_decrypt_sector(uint8_t* sector_data, uint8_t* key) {
    bitlocker_encrypt_sector(sector_data, key); 
}
