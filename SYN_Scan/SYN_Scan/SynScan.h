#ifndef SYN_SCAN_H
#define SYN_SCAN_H

#define USING_OPTIONS

#include "NetworkAdapter.h"

#define SRC_PORT	9999
#define WIN_SIZE	63443

DWORD WINAPI recvThread(LPVOID lpParam);

// pcap_loop �Ļص�����
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

// ���ڷ�װ���ݸ��̺߳����Ĳ���
typedef struct {
	pcap_t* adhandle;
	pcap_if_t* alldevs;
} PCAP_PARAM, *PPCAP_PARAM;


class SynScan :public NetworkAdapter {
public:
	SynScan(char* target_ip);
	~SynScan();
	void beginScan(); // ��Ŀ�������� 0~65535 �Ŷ˿ڽ���ɨ��

private:
	u_int m_target_ip; // Ŀ�������� IP
	u_int m_self_ip;
	u_char m_self_mac[MAC_LEN];
	u_char m_gate_mac[MAC_LEN];

	u_short cksum(u_short *p, int len);
	void make_syn_packet(u_char* packet, u_short dst_port);
};

#endif // SYN_SCAN_H
