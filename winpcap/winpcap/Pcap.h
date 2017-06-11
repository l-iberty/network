#ifndef PCAP_H
#define PCAP_H
#define HAVE_REMOTE
#define _CRT_SECURE_NO_WARNINGS
#include <pcap.h>
#include "headers.h"

/* �ص�����ԭ�� */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

/* ��ӡ������ֽ� */
void print_bytes(const u_char *buf, size_t len);

class Pcap {
public:
	Pcap();
	~Pcap();
	void print_devs();				/* ��ӡ�����������б� */
	void select_dev();				/* ѡ��һ�������� */
	bool open_adapter();		/* ��ѡ���������� */
	bool check_datalink();		/* �������������·�������Ƿ���� */
	void set_netmask();			/* �����������һ����ַ���������� */
	bool set_filter();				/* ���벢���ù����� */
	void start_cap();				/* ��ʼ�������ݰ� */

private:
	pcap_if_t *m_alldevs;		/* ��������������ṹ */
	pcap_if_t *m_d;					/* ����������ڵ㶨λָ�� */
	int m_devnum;					/* ����������*/
	pcap_t *m_adhandle;		/* pcap��� */
	u_int m_netmask;				/* 32λIP��ַ����������*/
	char *m_pkt_filter = "ip and tcp or udp";
	struct bpf_program m_fcode;		/* Structure for "pcap_compile()", "pcap_setfilter()" */
};

#endif // PCAP_H
