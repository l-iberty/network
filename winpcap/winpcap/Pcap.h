#ifndef PCAP_H
#define PCAP_H
#define HAVE_REMOTE
#define _CRT_SECURE_NO_WARNINGS
#include <pcap.h>
#include "headers.h"

/* 回调函数原型 */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

/* 打印缓冲池字节 */
void print_bytes(const u_char *buf, size_t len);

class Pcap {
public:
	Pcap();
	~Pcap();
	void print_devs();				/* 打印网络适配器列表 */
	void select_dev();				/* 选定一个适配器 */
	bool open_adapter();		/* 打开选定的适配器 */
	bool check_datalink();		/* 检查适配器的链路层数据是否可用 */
	void set_netmask();			/* 获得适配器第一个地址的子网掩码 */
	bool set_filter();				/* 编译并设置过滤器 */
	void start_cap();				/* 开始捕获数据包 */

private:
	pcap_if_t *m_alldevs;		/* 网络适配器链表结构 */
	pcap_if_t *m_d;					/* 适配器链表节点定位指针 */
	int m_devnum;					/* 适配器数量*/
	pcap_t *m_adhandle;		/* pcap句柄 */
	u_int m_netmask;				/* 32位IP地址的子网掩码*/
	char *m_pkt_filter = "ip and tcp or udp";
	struct bpf_program m_fcode;		/* Structure for "pcap_compile()", "pcap_setfilter()" */
};

#endif // PCAP_H
