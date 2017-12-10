#include "IcmpDetect.h"
#include <assert.h>

int aliveHostsNum;
extern HWND g_hListWnd;

DWORD WINAPI recvICMPThread(LPVOID lpParam)
{
	PPCAP_PARAM pParam = (PPCAP_PARAM)lpParam;
	u_int netmask;

	if (pParam->d->addresses != NULL)
	{
		// ��ýӿڵ�һ����ַ������
		netmask = ((struct sockaddr_in *)(pParam->d->addresses->netmask))->sin_addr.S_un.S_addr;
	}
	else
	{
		// ����ӿ�û�е�ַ������һ��C�������
		netmask = 0xffffff;
	}

	// ���������
	if (pcap_compile(pParam->adhandle, pParam->pfcode, pParam->filter, 1, netmask) <0)
	{
		AddString(g_hListWnd, "\nUnable to compile the packet filter. Check the syntax.\n");
		pcap_freealldevs(pParam->alldevs);
		return 1;
	}

	// ����
	if (pcap_setfilter(pParam->adhandle, pParam->pfcode) < 0)
	{
		AddString(g_hListWnd, "\nError setting the filter.\n");
		pcap_freealldevs(pParam->alldevs);
		return 1;
	}

	// �ͷ��豸�б���ʼ����
	pcap_freealldevs(pParam->alldevs);
	pcap_loop(pParam->adhandle, 0, packet_handler, NULL);
	
	return 0;
}

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	ip_header* iph;
	icmp_header *icmph;
	u_int ip_len;
	in_addr addr;
	char sz[256];

	iph = (ip_header*)(pkt_data + sizeof(ether_header));
	ip_len = (iph->ver_ihl & 0x0F) << 2;
	icmph = (icmp_header *)(pkt_data + sizeof(ether_header) + ip_len);
	// �ж��Ƿ��ǻظ��ҷ��� ICMP ����
	if (icmph->type == ICMP_REPLY &&
		icmph->id == (u_short)GetCurrentProcessId())
	{
		++aliveHostsNum;
		addr.S_un.S_addr = iph->saddr;
		sprintf(sz, "\n%s", inet_ntoa(addr));
		AddString(g_hListWnd, sz);
	}
}

/////////////////////////////////////////////// public ///////////////////////////////////////////////

IcmpDetect::IcmpDetect(u_int net, u_int netmask)
{
	m_net = net;
	m_netmask = netmask;
	m_hostnum = htonl(~netmask) - 1; // ��ȥ������ȫ0�������ַ��������ȫ1�Ĺ㲥��ַ
}

IcmpDetect::~IcmpDetect()
{

}

void IcmpDetect::beginDetect()
{
	u_char packet[sizeof(icmp_packet)];
	u_char src_mac[MAC_LEN];
	u_char dst_mac[MAC_LEN];
	u_int src_ip, netmask;
	u_int dst_ip;

	// ��ȡ���� MAC ��ַ
	if (!GetSelfMac(src_mac))
	{
		AddString(g_hListWnd, "\nError: cannot get physical address(MAC) of your PC!");
		return;
	}
	// ��ȡĬ�����ض�Ӧ�� MAC ��ַ (��·������ MAC ��ַ)
	GetMacOfDefaultGateway(dst_mac);

	// ��ȡ���� IP ��ַ����������
	if (getSelfIpAndMask(&src_ip, &netmask) == -1)
	{
		AddString(g_hListWnd, "\nError: cannot get ip addr and subnet mask of you PC!");
		return;
	}

	PCAP_PARAM param;
	param.adhandle = m_adhandle;
	param.alldevs = m_alldevs;
	param.d = m_d;
	param.filter = m_pkt_filter;
	param.pfcode = &m_fcode;

	assert(param.adhandle && param.alldevs && param.d);

	aliveHostsNum = 0;

	// ���������߳�
	DWORD dwThreadId;
	m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)recvICMPThread,
		(LPVOID)&param, 0, &dwThreadId);
	if (m_hThread == NULL)
	{
		AddString(g_hListWnd, "\nError: CreateThread failed!");
		return;
	}
	
	// ���� ICMP ������
	for (u_int host = 1;host < m_hostnum;host++)
	{
		dst_ip = m_net | htonl(host);
		make_icmp_packet(packet,
			src_mac, src_ip,
			dst_mac, dst_ip,
			ICMP_REQUEST, host);
		sendICMP(packet, sizeof(packet));
	}
}

void IcmpDetect::stopDetect()
{
	TerminateThread(m_hThread, 0);

	char sz[256];
	sprintf(sz, "Detection is stopped. Num of hosts: %d", aliveHostsNum);
	AddString(g_hListWnd, sz);
}

/////////////////////////////////////////////// private ///////////////////////////////////////////////

u_short IcmpDetect::cksum(u_short *p, int len) {
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

void IcmpDetect::make_icmp_packet(u_char* packet,
	u_char* src_mac, u_int src_ip,
	u_char* dst_mac, u_int dst_ip,
	u_char type, u_short seq)
{
	icmp_packet icmp_pkt;
	memset(&icmp_pkt, 0, sizeof(icmp_packet));

	// -----------------�����̫���ײ�-----------------
	memcpy(icmp_pkt.eh.daddr, dst_mac, MAC_LEN);
	memcpy(icmp_pkt.eh.saddr, src_mac, MAC_LEN);
	icmp_pkt.eh.prototype = htons(ETHPROTOCAL_IPV4);

	// -----------------��� IPv4 �ײ�-----------------
	icmp_pkt.iph.ver_ihl = 0x45;
	icmp_pkt.iph.tlen = htons(sizeof(icmp_pkt) - sizeof(ether_header));
	icmp_pkt.iph.ttl = 128;
	icmp_pkt.iph.proto = IPV4PROTOCOL_ICMP;
	icmp_pkt.iph.saddr = src_ip;
	icmp_pkt.iph.daddr = dst_ip;
	icmp_pkt.iph.cksum = cksum((u_short*)&icmp_pkt.iph, sizeof(ip_header));

	// -----------------��� ICMP �ײ�-----------------
	icmp_pkt.icmph.type = type;
	icmp_pkt.icmph.id = (u_short)GetCurrentProcessId();
	icmp_pkt.icmph.seq = htons(seq);
	memset(icmp_pkt.data, 0xCC, sizeof(icmp_pkt.data));
	icmp_pkt.icmph.cksum = cksum((u_short*)&icmp_pkt.icmph,
		sizeof(icmp_header) + sizeof(icmp_packet::data));

	memcpy(packet, &icmp_pkt, sizeof(icmp_pkt));
}

void IcmpDetect::sendICMP(u_char* packet, int len)
{
	if (m_adhandle != NULL)
	{
		if (pcap_sendpacket(m_adhandle, packet, len) == -1)
			AddString(g_hListWnd, "\nError: sending packet failed!");
	}
}
