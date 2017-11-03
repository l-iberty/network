#include "SynScan.h"

DWORD WINAPI recvThread(LPVOID lpParam)
{
	PPCAP_PARAM param = (PPCAP_PARAM)lpParam;
	pcap_freealldevs(param->alldevs);
	pcap_loop(param->adhandle, 0, packet_handler, NULL);
	return 0;
}

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	tcp_packet* recv_pkt = (tcp_packet*)pkt_data;
	u_short dst_port = htons(recv_pkt->th.dport);
	
	in_addr saddr;
	u_short src_port;
	if ((dst_port == SRC_PORT) &&
		(recv_pkt->th.flag & SYN) &&
		(recv_pkt->th.flag & ACK))
	{
		saddr.S_un.S_addr = recv_pkt->ih.saddr;
		src_port = htons(recv_pkt->th.sport);
		printf("\n%s -- %d", inet_ntoa(saddr), src_port);
	}
}

/////////////////////////////////////////////// public ///////////////////////////////////////////////

SynScan::SynScan(char* target_ip)
{
	m_target_ip = inet_addr(target_ip);

	DWORD dw;
	getSelfIpAndMask(&m_self_ip, (u_int*)&dw);
	GetSelfMac(m_self_mac);
	GetNetAddrOfDefaultGateway(&dw, m_gate_mac);
}

SynScan::~SynScan()
{

}

void SynScan::beginScan()
{
	u_short dst_port;
	u_char packet[sizeof(tcp_packet)];
	HANDLE hThread;
	DWORD dwTID;
	PCAP_PARAM param;

	param.adhandle = m_adhandle;
	param.alldevs = m_alldevs;
	hThread = CreateThread(NULL, 0, recvThread, (LPVOID)&param, 0, &dwTID);
	if (hThread == NULL)
	{
		printf("\nCreateThread Error! Scanning stopped!");
		return;
	}

	printf("\nscanning...");
	for (dst_port = 0;dst_port <= 65535;dst_port++)
	{
		make_syn_packet(packet, dst_port);
		pcap_sendpacket(m_adhandle, packet, sizeof(packet));
	}
}

/////////////////////////////////////////////// private ///////////////////////////////////////////////

u_short SynScan::cksum(u_short *p, int len) {
	int cksum = 0;
	u_short answer = 0;

	// 以16bits为单位累加
	while (len > 1) {
		u_short t = *p;
		cksum += *p++;
		len -= 2;
	}
	// 如果数据的字节数为奇数, 将最后一个字节视为16bits的高8bits, 低8bits补0, 继续累加
	if (len == 1) {
		answer = *(u_short *)p;
		cksum += answer;
	}
	// cksum是32bits的int, 而校验和需为16bits, 需将cksum的高16bits加到低16bits上
	cksum = (cksum >> 16) + (cksum & 0xffff);
	// 按位求反
	return (~(u_short)cksum);
}

void SynScan::make_syn_packet(u_char* packet, u_short dst_port)
{
	tcp_packet syn_pkt;
	memset(&syn_pkt, 0, sizeof(syn_pkt));

	// -----------------填充以太网首部-----------------
	memcpy(syn_pkt.eh.daddr, m_gate_mac, MAC_LEN);
	memcpy(syn_pkt.eh.saddr, m_self_mac, MAC_LEN);
	syn_pkt.eh.prototype = htons(ETHPROTOCAL_IPV4);

	// -----------------填充 IPv4 首部-----------------
	syn_pkt.ih.ver_ihl = 0x45;
	syn_pkt.ih.tlen = htons(sizeof(syn_pkt) - sizeof(ether_header));
	syn_pkt.ih.ttl = 128;
	syn_pkt.ih.proto = IPV4PROTOCOL_TCP;
	syn_pkt.ih.saddr = m_self_ip;
	syn_pkt.ih.daddr = m_target_ip;
	syn_pkt.ih.cksum = cksum((u_short*)&syn_pkt.ih, sizeof(ip_header));

	// -----------------填充 TCP 首部-----------------
	syn_pkt.th.sport = htons(SRC_PORT);
	syn_pkt.th.dport = htons(dst_port);
	syn_pkt.th.seq = htonl(rand());
	syn_pkt.th.ack = 0;
	syn_pkt.th.lenres = (sizeof(tcp_header) / 4) << 4; // len = sizeof(tcp_header) / 4, res = 0
	syn_pkt.th.flag = SYN;
	syn_pkt.th.win = htons(WIN_SIZE);
	syn_pkt.th.cksum = 0;
	syn_pkt.th.urp = 0;
	// 选项的填充参考 WireShark 的实际抓包结果
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

	// 使用伪首部计算 TCP 校验和
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
