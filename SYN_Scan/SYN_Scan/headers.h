#ifndef HEADERS_H
#define HEADERS_H

#define MAC_LEN				6		// MAC 地址, 48 bits = 6 bytes
#define IPV4_LEN			4		// IPV4 地址, 32 bits = 4 bytes
#define PADDING_LEN			18		// ARP 数据包的有效载荷长度
#define ICMP_DATA_LEN		32		// ICMP 数据包的有效载荷长度

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

// 以太网首部
typedef struct ether_header {
	u_char daddr[MAC_LEN];				// 目的MAC地址
	u_char saddr[MAC_LEN];				// 源MAC地址
	u_short prototype;					// 上层协议类型 (0x0800->IPv4, 0x0806->ARP)

#define ETHPROTOCAL_IPV4		0x0800 // 以太网上层协议类型: IPv4
#define ETHPROTOCAL_ARP			0x0806 // 以太网上层协议类型: ARP

} ether_header;

// 以太网 ARP 字段
typedef struct arp_header {
	// ARP 首部
	u_short arp_hrd;				// 硬件类型
#define HARD_ETHERNET		0x0001
	u_short arp_pro;				// 协议类型
	u_char arp_hln;					// 硬件地址长度
	u_char arp_pln;					// 协议地址长度
	u_short arp_op;					// 选项
#define ARP_REQUEST				0x0001 // ARP 请求
#define ARP_RESPONCE			0x0002 // ARP 应答
	u_char arp_shaddr[MAC_LEN];		// 发送者 MAC 地址
	u_int arp_spaddr;				// 发送者协议(IP)地址
	u_char arp_thaddr[MAC_LEN];		// 目标 MAC 地址
	u_int arp_tpaddr;				// 目标协议(IP)地址
} arp_header;

// ARP 包
typedef struct arp_packet {
	ether_header eh;				// 以太网首部
	arp_header ah;					// ARP 首部
	u_char padding[PADDING_LEN];
} arp_packet;

// IPv4 Header
typedef struct ip_header {
	u_char  ver_ihl;				// 版本 (4 bits) + 首部长度 (4 bits)
	u_char  tos;					// 服务类型(Type of service) 
	u_short tlen;					// 总长(Total length) 
	u_short identification;			// 标识(Identification)
	u_short flags_fo;				// 标志位(Flags) (3 bits) + 段偏移量(Fragment offset) (13 bits)
	u_char  ttl;					// 存活时间(Time to live)
	u_char  proto;					// 上层协议(Protocol)
#define IPV4PROTOCOL_ICMP	1
#define IPV4PROTOCOL_UDP	17
#define IPV4PROTOCOL_TCP	6
	u_short cksum;					// 首部校验和(Header checksum)
	u_int  saddr;				    // 源地址(Source address)
	u_int  daddr;					// 目的地址(Destination address)
}ip_header;

// ICMP Header
typedef struct icmp_header {
	u_char type;				// ICMP数据报类型
#define ICMP_REQUEST	8
#define ICMP_REPLY		0
	u_char code;				// 编码
	u_short cksum;				// 校验和
	u_short id;					// 标识(通常为当前进程pid)
	u_short seq;				// 序号
}icmp_header;

// ICMP 包
typedef struct icmp_packet {
	ether_header eh;				// 以太网首部
	ip_header ih;					// IPv4 首部
	icmp_header icmph;				// ICMP 首部
	u_char data[ICMP_DATA_LEN];
} icmp_packet;

// TCP Header
typedef struct tcp_header {
	u_short sport;				// 源端口号
	u_short dport;				// 目的端口号
	u_int seq;					// 序号
	u_int ack;					// 确认号
	u_char lenres;				// 4 bits 数据偏移(以 4 bytes 为单位), 4 bits 的保留字段
	u_char flag;				// 标志
// definition for flag[5:0]
#define FIN	1
#define SYN	(FIN << 1)
#define RST (SYN << 1)
#define PSH (RST << 1)
#define ACK (PSH << 1)
#define URG	(ACK << 1)
	u_short win;				// 窗口长度
	u_short cksum;				// 校验和
	u_short urp;				// 紧急指针

#ifdef USING_OPTIONS
	struct {
		struct {
			u_char kind;
			u_char len;
			u_short value;
		} mss;

		struct {
			u_char type;
		} opt1;

		struct {
			u_char kind;
			u_char len;
			u_char shift_count;
		} ws;

		struct {
			u_char type;
		} opt2;

		struct {
			u_char type;
		} opt3;

		struct {
			u_char kind;
			u_char len;
		} sack_perm;
	} options;
#endif
}tcp_header;

// UDP 首部
typedef struct udp_header {
	u_short sport;           	// 源端口(Source port)
	u_short dport;          	// 目的端口(Destination port)
	u_short len;				// UDP数据包长度(Datagram length)
	u_short cksum;         		// 校验和(Checksum)
}udp_header;

// TCP/UDP 伪首部
typedef struct psd_header {
	u_int saddr;				// 源IP
	u_int daddr;				// 目的IP
	u_char mbz;					// 不知道是什么, 设为0
	u_char proto;				// 协议类型
	u_short len;					// TCP/UDP报文长度
}psd_header;

// TCP 报文
typedef struct tcp_packet {
	ether_header eh;
	ip_header ih;
	tcp_header th;
}tcp_packet;

#endif // HEADERS_H
