#pragma once

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

/* 以太网首部 */
typedef struct ether_header {
	u_char daddr[6];				// 目的MAC地址
	u_char saddr[6];				// 源MAC地址
	u_short prototype;			// 上层协议类型 (0x0800->IP, 0x0806->ARP)
} ether_header;

/* IPv4 Header */
typedef struct ip_header {
	u_char  ver_ihl;					// 版本 (4 bits) + 首部长度 (4 bits)
	u_char  tos;						// 服务类型(Type of service) 
	u_short tlen;						// 总长(Total length) 
	u_short identification;		// 标识(Identification)
	u_short flags_fo;				// 标志位(Flags) (3 bits) + 段偏移量(Fragment offset) (13 bits)
	u_char  ttl;							// 存活时间(Time to live)
	u_char  proto;					// 上层协议(Protocol)
	u_short cksum;					// 首部校验和(Header checksum)
	u_char  saddr[4];	            // 源地址(Source address)
	u_char  daddr[4];			    // 目的地址(Destination address)
	u_int   op_pad;					// 选项与填充(Option + Padding)
}ip_header;

/* ICMP Header */
typedef struct icmp_header {
	unsigned char type;				// ICMP数据报类型
	unsigned char code;			// 编码
	unsigned short cksum;		// 校验和
	unsigned short id;				// 标识(通常为当前进程pid)
	unsigned short seq;				// 序号
}icmp_header;

/* TCP Header */
typedef struct tcp_header {
	u_short sport;			// 源端口号
	u_short dport;			// 目的端口号
	u_int seq;					// 序号
	u_int ack;					// 确认号
	u_char lenres;			// 4 bits 的数据偏移和 4 bits 的保留字段
	u_char flag;				// 标志
	u_short win;				// 窗口长度
	u_short cksum;			// 校验和
	u_short urp;				// 紧急指针
}tcp_header;

/* UDP 首部*/
typedef struct udp_header {
	u_short sport;           // 源端口(Source port)
	u_short dport;          // 目的端口(Destination port)
	u_short len;				// UDP数据包长度(Datagram length)
	u_short cksum;         // 校验和(Checksum)
}udp_header;