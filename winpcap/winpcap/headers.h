#pragma once

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

/* ��̫���ײ� */
typedef struct ether_header {
	u_char daddr[6];				// Ŀ��MAC��ַ
	u_char saddr[6];				// ԴMAC��ַ
	u_short prototype;			// �ϲ�Э������ (0x0800->IP, 0x0806->ARP)
} ether_header;

/* IPv4 Header */
typedef struct ip_header {
	u_char  ver_ihl;					// �汾 (4 bits) + �ײ����� (4 bits)
	u_char  tos;						// ��������(Type of service) 
	u_short tlen;						// �ܳ�(Total length) 
	u_short identification;		// ��ʶ(Identification)
	u_short flags_fo;				// ��־λ(Flags) (3 bits) + ��ƫ����(Fragment offset) (13 bits)
	u_char  ttl;							// ���ʱ��(Time to live)
	u_char  proto;					// �ϲ�Э��(Protocol)
	u_short cksum;					// �ײ�У���(Header checksum)
	u_char  saddr[4];	            // Դ��ַ(Source address)
	u_char  daddr[4];			    // Ŀ�ĵ�ַ(Destination address)
	u_int   op_pad;					// ѡ�������(Option + Padding)
}ip_header;

/* ICMP Header */
typedef struct icmp_header {
	unsigned char type;				// ICMP���ݱ�����
	unsigned char code;			// ����
	unsigned short cksum;		// У���
	unsigned short id;				// ��ʶ(ͨ��Ϊ��ǰ����pid)
	unsigned short seq;				// ���
}icmp_header;

/* TCP Header */
typedef struct tcp_header {
	u_short sport;			// Դ�˿ں�
	u_short dport;			// Ŀ�Ķ˿ں�
	u_int seq;					// ���
	u_int ack;					// ȷ�Ϻ�
	u_char lenres;			// 4 bits ������ƫ�ƺ� 4 bits �ı����ֶ�
	u_char flag;				// ��־
	u_short win;				// ���ڳ���
	u_short cksum;			// У���
	u_short urp;				// ����ָ��
}tcp_header;

/* UDP �ײ�*/
typedef struct udp_header {
	u_short sport;           // Դ�˿�(Source port)
	u_short dport;          // Ŀ�Ķ˿�(Destination port)
	u_short len;				// UDP���ݰ�����(Datagram length)
	u_short cksum;         // У���(Checksum)
}udp_header;