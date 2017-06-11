#include "Pcap.h"

Pcap::Pcap() {
	char errbuf[PCAP_ERRBUF_SIZE];

	/* ����豸�б� */
	if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &m_alldevs, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
}

Pcap::~Pcap() {

}

void Pcap::print_devs() {
	pcap_if_t *d;
	int i = 0;

	for (d = m_alldevs; d; d = d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0)
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
	else
		m_devnum = i; /* ��������������*/
}

void Pcap::select_dev() {
	int i, inum;

	printf("\nEnter the interface number (1-%d):", m_devnum);
	scanf("%d", &inum);

	if (inum < 1 || inum > m_devnum)
	{
		printf("\nInterface number out of range.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(m_alldevs);
		return;
	}

	/* ��ת����ѡ�豸 */
	for (m_d = m_alldevs, i = 0; i< inum - 1;m_d = m_d->next, i++);
}

bool Pcap::open_adapter() {
	char errbuf[PCAP_ERRBUF_SIZE];

	if ((m_adhandle = pcap_open(m_d->name,  // �豸��
		65536,		// Ҫ��׽�����ݰ��Ĳ��� 
				            // 65535��֤�ܲ��񵽲�ͬ������·���ϵ�ÿ�����ݰ���ȫ������
		PCAP_OPENFLAG_PROMISCUOUS,            // ����ģʽ
		1000,      // ��ȡ��ʱʱ��
		NULL,      // Զ�̻�����֤
		errbuf     // ���󻺳��
		)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", m_d->name);
		/* �ͷ��豸�б� */
		pcap_freealldevs(m_alldevs);
		return false;
	}
	return true;
}

bool Pcap::check_datalink() {
	/* ���������·�㣬Ϊ�˼򵥣�ֻ������̫�� */
	/* pcap_datalink ��������������·�� */
	if (pcap_datalink(m_adhandle) != DLT_EN10MB)
	{
		fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(m_alldevs);
		return false;
	}
	return true;
}

void Pcap::set_netmask() {
	if (m_d->addresses != NULL)
		/* ��ýӿڵ�һ����ַ������ */
		m_netmask = ((struct sockaddr_in *)(m_d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* ����ӿ�û�е�ַ����ô���Ǽ���һ��C������� */
		m_netmask = 0xffffff;
}

bool Pcap::set_filter() {
	//���������
	if (pcap_compile(m_adhandle, &m_fcode, m_pkt_filter, 1, m_netmask) <0)
	{
		fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(m_alldevs);
		return false;
	}

	//���ù�����
	if (pcap_setfilter(m_adhandle, &m_fcode)<0)
	{
		fprintf(stderr, "\nError setting the filter.\n");
		/* �ͷ��豸�б� */
		pcap_freealldevs(m_alldevs);
		return false;
	}
	return true;
}

void Pcap::start_cap() {
	printf("\nlistening on %s...\n", m_d->description);

	/* �ͷ��豸�б� */
	pcap_freealldevs(m_alldevs);

	/* ��ʼ��׽ */
	pcap_loop(m_adhandle, 0, packet_handler, NULL);
}

/* �ص����������յ�ÿһ�����ݰ�ʱ�ᱻlibpcap������ */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	struct tm *ltime;
	char timestr[16];
	ip_header *ih;
	tcp_header *th;
	udp_header *uh;
	ether_header *eh;
	u_int ip_len;
	u_short sport, dport;
	time_t local_tv_sec;

	/* ��ʱ���ת���ɿ�ʶ��ĸ�ʽ */
	local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof(timestr), "%H:%M:%S", ltime);

	/* ��ӡ���ݰ���ʱ����ͳ��� */
	printf("%s.%d len:%d\n", timestr, header->ts.tv_usec, header->len);

	/* ��ӡ���ݰ������ֽ� */
	print_bytes(pkt_data, header->len);

	/* �����̫���ײ� */
	eh = (ether_header *)pkt_data;
	printf("%.2x-%.2x-%.2x-%.2x-%.2x-%.2x -> %.2x-%.2x-%.2x-%.2x-%.2x-%.2x\n",
		eh->saddr[0], eh->saddr[1], eh->saddr[2], eh->saddr[3], eh->saddr[4], eh->saddr[5],
		eh->daddr[0], eh->daddr[1], eh->daddr[2], eh->daddr[3], eh->daddr[4], eh->daddr[5]);

	/* ���IP���ݰ�ͷ����λ�� */
	ih = (ip_header *)(pkt_data +
		14); // ��̫���ײ�����

	/* IP�ײ��ĳ��� */
	ip_len = (ih->ver_ihl & 0x0f) << 2;
	
	/* �жϴ����Э�� */
	char proto[15];
	switch (ih->proto)
	{
	case IPPROTO_TCP:
		/*  ���TCP�ײ���λ�� */
		th = (tcp_header *)((u_char *)ih + ip_len);

		/* �������ֽ�����ת���������ֽ����� */
		sport = ntohs(th->sport);
		dport = ntohs(th->dport);

		strcpy(proto, "<TCP>");
		break;
	case IPPROTO_UDP:
		/* ���UDP�ײ���λ�� */
		uh = (udp_header *)((u_char *)ih + ip_len);

		/* �������ֽ�����ת���������ֽ����� */
		sport = ntohs(uh->sport);
		dport = ntohs(uh->dport);

		strcpy(proto, "<UDP>");
		break;
	default:
		strcpy(proto, "<Other proto>");
		sport = dport = 0;
	}

	/* ��ӡIP��ַ�Ͷ˿� */
	printf("%s %d.%d.%d.%d.%d -> %d.%d.%d.%d.%d\n\n",
		proto,
		ih->saddr[0], ih->saddr[1], ih->saddr[2], ih->saddr[3],
		sport,
		ih->daddr[0], ih->daddr[1], ih->daddr[2], ih->daddr[3],
		dport);
}

void print_bytes(const u_char *buf, size_t len)
{
	printf("--------------------- BEGIN ---------------------\n");
	for (size_t i = 0;i < len;i++)
	{
		printf("%.2x ", buf[i]);
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n--------------------- END ---------------------\n");
}