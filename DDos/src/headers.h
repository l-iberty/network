#ifndef DDOS_HEADERS_H
#define DDOS_HEADERS_H

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// TCP/UDP伪首部
struct psd_hdr{
    u32 saddr;
    u32 daddr;
    u8 mbz;	// 不知道是什么, 设为0
    u8 proto;	// 协议类型
    u8 len;	// TCP/UDP报文长度
};

struct tcp_hdr {
    u16 tcp_sport;	 // 源端口号
    u16 tcp_dport;	 // 目的端口号
    u32 tcp_seq;	 // 序号
    u32 tcp_ack;	 // 确认号
    u8 tcp_lenres;	 // 4 bits 的数据偏移和 4 bits 的保留字段
    u8 tcp_flag;	 // 标志
    u16 tcp_win;	 // 窗口长度
    u16 tcp_checksum;	 // 校验和
    u16 tcp_urp;	 // 紧急指针
};

struct udp_hdr {
    u16 udp_sport;	// 源端口号
    u16 udp_dport;	// 目的端口号
    u16 udp_len;	// 长度
    u16 udp_checksum;	// 校验和
};

struct ip_hdr {
    u8 ip_verlen;		 // 4位版本号, 4位首部长度
    u8 ip_tos;			 // 8位服务类型TOS
    u16 ip_totallen;		 // 16位数据报总长度（字节）
    u16 ip_id;			 // 16位标识
    u16 ip_frag_and_offset;	 // 3位标志位, 13位偏移
    u8 ip_ttl;			 // 8位生存时间 TTL
    u8 ip_proto;		 // 8位协议 (TCP, UDP 或其他)
    u16 ip_checksum;		 // 16位IP首部校验和
    u32 ip_srcip;		 // 32位源IP地址
    u32 ip_dstip;		 // 32位目的IP地址
};

#endif //DDOS_HEADERS_H
