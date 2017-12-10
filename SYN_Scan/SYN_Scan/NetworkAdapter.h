#ifndef NETWORK_ADAPTER_H
#define NETWORK_ADAPTER_H

#include "Pcap.h"
#include "headers.h"
#include <Shlwapi.h>
#include <Packet32.h>
#include <Windows.h>
#include <ntddndis.h>
#include <Iphlpapi.h>

#define MAX_SNAP_LEN	65536

class NetworkAdapter {
public:
	NetworkAdapter();
	~NetworkAdapter();

	pcap_t* openAdapter();
	int getSelfIpAndMask(u_int *pIp, u_int *pMask);

	BOOLEAN GetSelfMac(PUCHAR MacAddr);
	VOID GetNetAddrOfDefaultGateway(PDWORD pIpAddr, PUCHAR MacAddr);

protected:
	pcap_if_t *m_alldevs;					// ��������������ṹ
	pcap_if_t *m_d;								// ����������ڵ㶨λָ��
	int m_devnum;								// ���������� (unused)
	pcap_t *m_adhandle;					// pcap���
	pcap_addr_t *m_paddr;				// ������ַ�ṹ
	char m_AdapterName[PCAP_BUF_SIZE];

private:
	int getAllDevs();
	void getAdapterParams();
};

#endif // NETWORK_ADAPTER_H