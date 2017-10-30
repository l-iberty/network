#include "NetworkAdapter.h"

/////////////////////////////////////////////// public ///////////////////////////////////////////////

NetworkAdapter::NetworkAdapter()
{
	m_alldevs = NULL;
	m_d = NULL;
	m_adhandle = NULL;
	m_paddr = NULL;

	if (getAllDevs() != -1)
	{
		getAdapterParams();
		m_adhandle = openAdapter();
	}
	else
	{
		printf("\nError: adapters not found!");
	}
}

NetworkAdapter::~NetworkAdapter()
{
}

pcap_t* NetworkAdapter::openAdapter()
{
	char errbuf[PCAP_ERRBUF_SIZE];

	pcap_t* adhandle = pcap_open(m_AdapterName, // �豸��
		MAX_SNAP_LEN,	// �ɴ�������ݰ�����
		PCAP_OPENFLAG_PROMISCUOUS, // ����ģʽ
		1000,  // ��ȡ��ʱʱ��
		NULL, // Զ�̻�����֤
		errbuf); // ���󻺳��

	if (adhandle == NULL)
	{
		printf("\nError: cannot open the adapter: %s", errbuf);
		pcap_freealldevs(m_alldevs);
	}
	return adhandle;
}

/**
* ��ȡһ�����õ�������������Ӧ�� ip ��ַ(���� ip ��ַ)��������.
* return 0 if succeeded, or (-1) if failed
*/
int NetworkAdapter::getSelfIpAndMask(u_int *pIp, u_int *pMask)
{
	u_int ip, mask;
	BOOLEAN bOk = FALSE;
	for (m_paddr = m_d->addresses;m_paddr;m_paddr = m_paddr->next)
	{
		ip = ((struct sockaddr_in *)(m_paddr->addr))->sin_addr.S_un.S_addr;
		mask = ((struct sockaddr_in *)(m_paddr->netmask))->sin_addr.S_un.S_addr;

		if (ip && mask)
		{
			*pIp = ip;
			*pMask = mask;
			bOk = TRUE;
			break;
		}
	}
	if (!bOk)
	{
		pcap_freealldevs(m_alldevs);
		return 1;
	}
	return 0;
}

BOOLEAN NetworkAdapter::GetSelfMac(PUCHAR MacAddr)
{
	LPADAPTER lpAdapter = PacketOpenAdapter(m_AdapterName + 8); // `+8` ����"rpcap:\\"
	if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE))
		return FALSE;

	PPACKET_OID_DATA pOidData = (PPACKET_OID_DATA)malloc(
		sizeof(PACKET_OID_DATA) + MAC_LEN); // �鿴�ṹ�嶨�壬���MAC��ַ�ĳ��ȣ����֪��'+6'�ĺ���
	if (pOidData == NULL)
	{
		PacketCloseAdapter(lpAdapter);
		return FALSE;
	}

	// Retrieve the adapter MAC querying the NIC driver
	pOidData->Oid = OID_802_3_CURRENT_ADDRESS; // ��ȡ MAC ��ַ
	pOidData->Length = MAC_LEN;
	memset(pOidData->Data, 0, MAC_LEN);

	BOOLEAN bOk = PacketRequest(lpAdapter, FALSE, pOidData);
	if (bOk)
		memcpy(MacAddr, pOidData->Data, MAC_LEN);

	free(pOidData);
	PacketCloseAdapter(lpAdapter);
	return bOk;
}

VOID NetworkAdapter::GetMacOfDefaultGateway(PUCHAR MacAddr)
{
	PMIB_IPNETTABLE pIpNetTable = NULL;
	ULONG dwSize = 0;

	if (GetIpNetTable(pIpNetTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
	{
		// The required size is returned in the `dwSize`
		pIpNetTable = (PMIB_IPNETTABLE)malloc(dwSize);
		GetIpNetTable(pIpNetTable, &dwSize, FALSE);
	}

	for (DWORD i = 0;i < pIpNetTable->dwNumEntries;i++)
	{
		// ��һ�� dwPhysAddrLen ��Ϊ 0 �������Ĭ������
		if (pIpNetTable->table[i].dwPhysAddrLen == MAC_LEN)
		{
			memcpy(MacAddr, pIpNetTable->table[i].bPhysAddr, MAC_LEN);
			break;
		}
	}
	free(pIpNetTable);
}

/////////////////////////////////////////////// private ///////////////////////////////////////////////

int NetworkAdapter::getAllDevs()
{
	int ret = -1;
	char errbuf[PCAP_ERRBUF_SIZE];
	ret = pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &m_alldevs, errbuf);
	if (ret == -1)
	{
		printf("\nError: pcap_findalldevs_ex: %s", errbuf);
	}
	return ret; // return (-1) if failed
}

/**
* ��ȡ����������:
* 1. ��`m_d`��λ��һ�����õ�����������
* 2. ��ȡ����������`m_devnum`
* 3. ��ȡ`m_d`ָ���������������, ���������`m_AdapterName`
*/
void NetworkAdapter::getAdapterParams()
{
	int count = 0;
	bool isLocated = false;
	pcap_if_t *d;
	for (d = m_alldevs; d; d = d->next, count++)
	{
		if (!isLocated && d->description)
		{
			// ������Ϣ�к��йؼ��� "Microsoft" ����������Ϊ���õ�������
			if (StrStrIA(d->description, "Microsoft"))
			{
				m_d = d; // 1. ��λ�����õ�����������
				isLocated = true;
			}
		}
	}
	if (count == 0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
	}
	else
	{
		m_devnum = count; // 2. ��������������
		strcpy(m_AdapterName, m_d->name); // 3. ��������������
	}
}
