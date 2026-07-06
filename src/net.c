#include "net.h"
#include "e1000.h"
#include "vga.h"
#include "string.h"

static uint8_t cloud_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
static uint8_t cloud_ip[4] = {192, 168, 1, 100};

// 체크섬 계산 (IP/ICMP 공용)
uint16_t net_checksum(void* data, uint32_t len) {
    uint32_t sum = 0;
    uint16_t* ptr = (uint16_t*)data;
    while (len > 1) { sum += *ptr++; len -= 2; }
    if (len > 0) sum += *(uint8_t*)ptr;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return ~((uint16_t)sum);
}

void net_handle_packet(uint8_t* data, uint32_t len) {
    eth_header_t* eth = (eth_header_t*)data;
    
    // ARP 요청 처리
    if (eth->ethertype == 0x0608) { // 0x0806 in Little Endian
        arp_packet_t* arp = (arp_packet_t*)(data + sizeof(eth_header_t));
        if (arp->opcode == 0x0100 && memcmp(arp->target_ip, cloud_ip, 4) == 0) {
            // ARP Reply 생성
            uint8_t reply[64];
            eth_header_t* r_eth = (eth_header_t*)reply;
            memcpy(r_eth->dest_mac, arp->sender_mac, 6);
            memcpy(r_eth->src_mac, cloud_mac, 6);
            r_eth->ethertype = 0x0608;
            
            arp_packet_t* r_arp = (arp_packet_t*)(reply + sizeof(eth_header_t));
            r_arp->hw_type = 0x0100; r_arp->proto_type = 0x0008;
            r_arp->hw_len = 6; r_arp->proto_len = 4;
            r_arp->opcode = 0x0200; // Reply
            memcpy(r_arp->sender_mac, cloud_mac, 6);
            memcpy(r_arp->sender_ip, cloud_ip, 4);
            memcpy(r_arp->dest_mac, arp->sender_mac, 6);
            memcpy(r_arp->dest_ip, arp->sender_ip, 4);
            
            e1000_send_packet(reply, 42);
            vga_print("[NET] Replied to ARP request.\n");
        }
    }
    // ICMP (Ping) 요청 처리
    else if (eth->ethertype == 0x0008) { // 0x0800 in Little Endian
        ipv4_header_t* ip = (ipv4_header_t*)(data + sizeof(eth_header_t));
        if (ip->protocol == 1) { // ICMP
            icmp_header_t* icmp = (icmp_header_t*)(data + sizeof(eth_header_t) + (ip->version_ihl & 0x0F) * 4);
            if (icmp->type == 8) { // Echo Request
                vga_print("[NET] Received Ping. Sending Reply.\n");
                
                uint8_t reply[len];
                memcpy(reply, data, len);
                eth_header_t* r_eth = (eth_header_t*)reply;
                memcpy(r_eth->dest_mac, eth->src_mac, 6);
                memcpy(r_eth->src_mac, cloud_mac, 6);
                
                ipv4_header_t* r_ip = (ipv4_header_t*)(reply + sizeof(eth_header_t));
                memcpy(r_ip->dest_ip, ip->src_ip, 4);
                memcpy(r_ip->src_ip, cloud_ip, 4);
                r_ip->checksum = 0;
                r_ip->checksum = net_checksum(r_ip, (r_ip->version_ihl & 0x0F) * 4);
                
                icmp_header_t* r_icmp = (icmp_header_t*)(reply + sizeof(eth_header_t) + (r_ip->version_ihl & 0x0F) * 4);
                r_icmp->type = 0; // Echo Reply
                r_icmp->checksum = 0;
                r_icmp->checksum = net_checksum(r_icmp, len - sizeof(eth_header_t) - (r_ip->version_ihl & 0x0F) * 4);
                
                e1000_send_packet(reply, len);
            }
        }
    }
}
