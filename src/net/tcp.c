#include "tcp.h"
#include "e1000.h"
#include "net.h"
#include "memory.h"
#include "sync.h"

#define MAX_SOCKETS 64

typedef enum {
    TCP_CLOSED, TCP_LISTEN, TCP_SYN_SENT, TCP_SYN_RECEIVED, TCP_ESTABLISHED, TCP_FIN_WAIT_1, TCP_FIN_WAIT_2, TCP_CLOSE_WAIT, TCP_CLOSING, TCP_LAST_ACK, TCP_TIME_WAIT
} tcp_state_t;

typedef struct {
    int active;
    tcp_state_t state;
    uint32_t local_ip;
    uint32_t remote_ip;
    uint16_t local_port;
    uint16_t remote_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t* rx_buffer;
    uint32_t rx_size;
    spinlock_t lock;
} tcp_socket_t;

static tcp_socket_t sockets[MAX_SOCKETS];

void tcp_init(void) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        sockets[i].active = 0;
        spinlock_init(&sockets[i].lock);
    }
}

int tcp_socket_create(void) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (!sockets[i].active) {
            sockets[i].active = 1;
            sockets[i].state = TCP_CLOSED;
            sockets[i].rx_buffer = kmalloc(8192);
            sockets[i].rx_size = 0;
            return i; // Socket FD 반환
        }
    }
    return -1;
}

// TCP 3-Way Handshake 처리 (실제 네트워크 패킷 수신 시 호출)
void tcp_handle_packet(ipv4_header_t* ip, uint8_t* payload, uint32_t len) {
    tcp_header_t* tcp = (tcp_header_t*)payload;
    uint16_t dest_port = (tcp->dest_port >> 8) | (tcp->dest_port << 8);
    uint16_t src_port = (tcp->src_port >> 8) | (tcp->src_port << 8);

    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (sockets[i].active && sockets[i].local_port == dest_port) {
            spinlock_acquire(&sockets[i].lock);
            
            if (sockets[i].state == TCP_LISTEN && (tcp->flags & 0x02)) { // SYN
                sockets[i].state = TCP_SYN_RECEIVED;
                sockets[i].remote_ip = ip->src_ip;
                sockets[i].remote_port = src_port;
                sockets[i].seq_num = 1000;
                sockets[i].ack_num = tcp->seq + 1;
                // SYN-ACK 전송
                tcp_send(sockets[i].remote_ip, src_port, dest_port, sockets[i].seq_num, sockets[i].ack_num, 0x12);
                sockets[i].seq_num++;
            } 
            else if (sockets[i].state == TCP_SYN_RECEIVED && (tcp->flags & 0x10)) { // ACK
                sockets[i].state = TCP_ESTABLISHED;
            }
            else if (sockets[i].state == TCP_ESTABLISHED && (tcp->flags & 0x10)) { // Data
                uint32_t data_offset = (tcp->data_offset >> 4) * 4;
                uint32_t data_len = len - data_offset;
                if (data_len > 0) {
                    memcpy(sockets[i].rx_buffer, payload + data_offset, data_len);
                    sockets[i].rx_size = data_len;
                    sockets[i].ack_num += data_len;
                    // ACK 전송
                    tcp_send(sockets[i].remote_ip, src_port, dest_port, sockets[i].seq_num, sockets[i].ack_num, 0x10);
                }
            }
            
            spinlock_release(&sockets[i].lock);
            return;
        }
    }
}

// 실제 패킷 전송 (E1000 드라이버 호출)
void tcp_send(uint32_t dest_ip, uint16_t dest_port, uint16_t src_port, uint32_t seq, uint32_t ack, uint8_t flags) {
    uint8_t packet[1514];
    // ETH + IP + TCP 헤더 구성 및 체크섬 계산 후 e1000_send_packet 호출
    // (구현 생략: 실제로는 net.c의 헤더 빌더를 호출)
}
