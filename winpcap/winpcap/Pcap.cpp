#include "Pcap.h"

Pcap::Pcap() {
	char errbuf[PCAP_ERRBUF_SIZE];

	/* 获得设备列表 */
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
		m_devnum = i; /* 网络适配器数量*/
}

void Pcap::select_dev() {
	int i, inum;

	printf("\nEnter the interface number (1-%d):", m_devnum);
	scanf("%d", &inum);

	if (inum < 1 || inum > m_devnum)
	{
		printf("\nInterface number out of range.\n");
		/* 释放设备列表 */
		pcap_freealldevs(m_alldevs);
		return;
	}

	/* 跳转到已选设备 */
	for (m_d = m_alldevs, i = 0; i< inum - 1;m_d = m_d->next, i++);
}

bool Pcap::open_adapter() {
	char errbuf[PCAP_ERRBUF_SIZE];

	if ((m_adhandle = pcap_open(m_d->name,  // 设备名
		65536,		// 要捕捉的数据包的部分 
				            // 65535保证能捕获到不同数据链路层上的每个数据包的全部内容
		PCAP_OPENFLAG_PROMISCUOUS,            // 混杂模式
		1000,      // 读取超时时间
		NULL,      // 远程机器验证
		errbuf     // 错误缓冲池
		)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", m_d->name);
		/* 释放设备列表 */
		pcap_freealldevs(m_alldevs);
		return false;
	}
	return true;
}

bool Pcap::check_datalink() {
	/* 检查数据链路层，为了简单，只考虑以太网 */
	/* pcap_datalink 返回适配器的链路层 */
	if (pcap_datalink(m_adhandle) != DLT_EN10MB)
	{
		fprintf(stderr, "\nThis program works only on Ethernet networks.\n");
		/* 释放设备列表 */
		pcap_freealldevs(m_alldevs);
		return false;
	}
	return true;
}

void Pcap::set_netmask() {
	if (m_d->addresses != NULL)
		/* 获得接口第一个地址的掩码 */
		m_netmask = ((struct sockaddr_in *)(m_d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* 如果接口没有地址，那么我们假设一个C类的掩码 */
		m_netmask = 0xffffff;
}

bool Pcap::set_filter() {
	//编译过滤器
	if (pcap_compile(m_adhandle, &m_fcode, m_pkt_filter, 1, m_netmask) <0)
	{
		fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
		/* 释放设备列表 */
		pcap_freealldevs(m_alldevs);
		return false;
	}

	//设置过滤器
	if (pcap_setfilter(m_adhandle, &m_fcode)<0)
	{
		fprintf(stderr, "\nError setting the filter.\n");
		/* 释放设备列表 */
		pcap_freealldevs(m_alldevs);
		return false;
	}
	return true;
}

void Pcap::start_cap() {
	printf("\nlistening on %s...\n", m_d->description);

	/* 释放设备列表 */
	pcap_freealldevs(m_alldevs);

	/* 开始捕捉 */
	pcap_loop(m_adhandle, 0, packet_handler, NULL);
}

/* 回调函数，当收到每一个数据包时会被libpcap所调用 */
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

	/* 将时间戳转换成可识别的格式 */
	local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof(timestr), "%H:%M:%S", ltime);

	/* 打印数据包的时间戳和长度 */
	printf("%s.%d len:%d\n", timestr, header->ts.tv_usec, header->len);

	/* 打印数据包所有字节 */
	print_bytes(pkt_data, header->len);

	/* 获得以太网首部 */
	eh = (ether_header *)pkt_data;
	printf("%.2x-%.2x-%.2x-%.2x-%.2x-%.2x -> %.2x-%.2x-%.2x-%.2x-%.2x-%.2x\n",
		eh->saddr[0], eh->saddr[1], eh->saddr[2], eh->saddr[3], eh->saddr[4], eh->saddr[5],
		eh->daddr[0], eh->daddr[1], eh->daddr[2], eh->daddr[3], eh->daddr[4], eh->daddr[5]);

	/* 获得IP数据包头部的位置 */
	ih = (ip_header *)(pkt_data +
		14); // 以太网首部长度

	/* IP首部的长度 */
	ip_len = (ih->ver_ihl & 0x0f) << 2;
	
	/* 判断传输层协议 */
	char proto[15];
	switch (ih->proto)
	{
	case IPPROTO_TCP:
		/*  获得TCP首部的位置 */
		th = (tcp_header *)((u_char *)ih + ip_len);

		/* 将网络字节序列转换成主机字节序列 */
		sport = ntohs(th->sport);
		dport = ntohs(th->dport);

		strcpy(proto, "<TCP>");
		break;
	case IPPROTO_UDP:
		/* 获得UDP首部的位置 */
		uh = (udp_header *)((u_char *)ih + ip_len);

		/* 将网络字节序列转换成主机字节序列 */
		sport = ntohs(uh->sport);
		dport = ntohs(uh->dport);

		strcpy(proto, "<UDP>");
		break;
	default:
		strcpy(proto, "<Other proto>");
		sport = dport = 0;
	}

	/* 打印IP地址和端口 */
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