#include "tcp.h"
#include "e1000.h"
#include "net.h"

typedef enum {
    TCP_CLOSED, TCP_LISTEN, TCP_SYN_SENT, TCP_SYN_RECEIVED, TCP_ESTABLISHED
} tcp_state_t;

static tcp_state_t state = TCP_CLOSED;

void tcp_handle_packet(ipv4_header_t* ip, uint8_t* payload, uint32_t len) {
    tcp_header_t* tcp = (tcp_header_t*)payload;
    uint8_t flags = tcp->flags;
    
    // 3-Way Handshake 상태 머신
    if (state == TCP_LISTEN && (flags & 0x02)) { // SYN
        state = TCP_SYN_RECEIVED;
        // SYN-ACK 전송 로직 (체크섬 계산 및 E1000 송신)
        tcp_send(ip->src_ip, tcp->src_port, 0, 0, 0x12); // SYN+ACK
    } else if (state == TCP_SYN_RECEIVED && (flags & 0x10)) { // ACK
        state = TCP_ESTABLISHED;
        // 연결 완료, 앱에 소켓 사용 가능 알림
    } else if (state == TCP_ESTABLISHED && (flags & 0x10)) { // Data ACK
        // 페이로드 처리 (HTTP 요청 등)
    }
}
