#ifndef TCP_H
#define TCP_H
#include "types.h"
#include "net.h" // ipv4_header_t 등을 사용하기 위해

typedef struct {
    uint16_t src_port;
    uint16_t dest_port;
    uint32_t seq;
    uint32_t ack;
    uint8_t data_offset;
    uint8_t flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} __attribute__((packed)) tcp_header_t;

void tcp_handle_packet(ipv4_header_t* ip, uint8_t* payload, uint32_t len);
void tcp_send(uint32_t dest_ip, uint16_t dest_port, uint16_t src_port, uint32_t seq, uint32_t ack, uint8_t flags);

#endif
