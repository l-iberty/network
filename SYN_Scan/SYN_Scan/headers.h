#ifndef HEADERS_H
#define HEADERS_H

#define MAC_LEN				6		// MAC ��ַ, 48 bits = 6 bytes
#define IPV4_LEN			4		// IPV4 ��ַ, 32 bits = 4 bytes
#define PADDING_LEN			18		// ARP ���ݰ�����Ч�غɳ���
#define ICMP_DATA_LEN		32		// ICMP ���ݰ�����Ч�غɳ���

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;

// ��̫���ײ�
typedef struct ether_header {
	u_char daddr[MAC_LEN];				// Ŀ��MAC��ַ
	u_char saddr[MAC_LEN];				// ԴMAC��ַ
	u_short prototype;					// �ϲ�Э������ (0x0800->IPv4, 0x0806->ARP)

#define ETHPROTOCAL_IPV4		0x0800 // ��̫���ϲ�Э������: IPv4
#define ETHPROTOCAL_ARP			0x0806 // ��̫���ϲ�Э������: ARP

} ether_header;

// ��̫�� ARP �ֶ�
typedef struct arp_header {
	// ARP �ײ�
	u_short arp_hrd;				// Ӳ������
#define HARD_ETHERNET		0x0001
	u_short arp_pro;				// Э������
	u_char arp_hln;					// Ӳ����ַ����
	u_char arp_pln;					// Э���ַ����
	u_short arp_op;					// ѡ��
#define ARP_REQUEST				0x0001 // ARP ����
#define ARP_RESPONCE			0x0002 // ARP Ӧ��
	u_char arp_shaddr[MAC_LEN];		// ������ MAC ��ַ
	u_int arp_spaddr;				// ������Э��(IP)��ַ
	u_char arp_thaddr[MAC_LEN];		// Ŀ�� MAC ��ַ
	u_int arp_tpaddr;				// Ŀ��Э��(IP)��ַ
} arp_header;

// ARP ��
typedef struct arp_packet {
	ether_header eh;				// ��̫���ײ�
	arp_header ah;					// ARP �ײ�
	u_char padding[PADDING_LEN];
} arp_packet;

// IPv4 Header
typedef struct ip_header {
	u_char  ver_ihl;				// �汾 (4 bits) + �ײ����� (4 bits)
	u_char  tos;					// ��������(Type of service) 
	u_short tlen;					// �ܳ�(Total length) 
	u_short identification;			// ��ʶ(Identification)
	u_short flags_fo;				// ��־λ(Flags) (3 bits) + ��ƫ����(Fragment offset) (13 bits)
	u_char  ttl;					// ���ʱ��(Time to live)
	u_char  proto;					// �ϲ�Э��(Protocol)
#define IPV4PROTOCOL_ICMP	1
#define IPV4PROTOCOL_UDP	17
#define IPV4PROTOCOL_TCP	6
	u_short cksum;					// �ײ�У���(Header checksum)
	u_int  saddr;				    // Դ��ַ(Source address)
	u_int  daddr;					// Ŀ�ĵ�ַ(Destination address)
}ip_header;

// ICMP Header
typedef struct icmp_header {
	u_char type;				// ICMP���ݱ�����
#define ICMP_REQUEST	8
#define ICMP_REPLY		0
	u_char code;				// ����
	u_short cksum;				// У���
	u_short id;					// ��ʶ(ͨ��Ϊ��ǰ����pid)
	u_short seq;				// ���
}icmp_header;

// ICMP ��
typedef struct icmp_packet {
	ether_header eh;				// ��̫���ײ�
	ip_header ih;					// IPv4 �ײ�
	icmp_header icmph;				// ICMP �ײ�
	u_char data[ICMP_DATA_LEN];
} icmp_packet;

// TCP Header
typedef struct tcp_header {
	u_short sport;				// Դ�˿ں�
	u_short dport;				// Ŀ�Ķ˿ں�
	u_int seq;					// ���
	u_int ack;					// ȷ�Ϻ�
	u_char lenres;				// 4 bits ����ƫ��(�� 4 bytes Ϊ��λ), 4 bits �ı����ֶ�
	u_char flag;				// ��־
// definition for flag[5:0]
#define FIN	1
#define SYN	(FIN << 1)
#define RST (SYN << 1)
#define PSH (RST << 1)
#define ACK (PSH << 1)
#define URG	(ACK << 1)
	u_short win;				// ���ڳ���
	u_short cksum;				// У���
	u_short urp;				// ����ָ��

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

// UDP �ײ�
typedef struct udp_header {
	u_short sport;           	// Դ�˿�(Source port)
	u_short dport;          	// Ŀ�Ķ˿�(Destination port)
	u_short len;				// UDP���ݰ�����(Datagram length)
	u_short cksum;         		// У���(Checksum)
}udp_header;

// TCP/UDP α�ײ�
typedef struct psd_header {
	u_int saddr;				// ԴIP
	u_int daddr;				// Ŀ��IP
	u_char mbz;					// ��֪����ʲô, ��Ϊ0
	u_char proto;				// Э������
	u_short len;					// TCP/UDP���ĳ���
}psd_header;

// TCP ����
typedef struct tcp_packet {
	ether_header eh;
	ip_header ih;
	tcp_header th;
}tcp_packet;

#endif // HEADERS_H
