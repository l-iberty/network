#include "NetworkAdapter.h"

/////////////////////////////////////////////// public ///////////////////////////////////////////////

NetworkAdapter::NetworkAdapter()
{
	m_alldevs = NULL;
	m_d = NULL;
	m_adhandle = NULL;
	m_paddr = NULL;

	if (getAllDevs() != -1)
		getAdapterParams();
	else
		printf("\nError: adapters not found!");
}

NetworkAdapter::~NetworkAdapter()
{
}

pcap_t* NetworkAdapter::openAdapter()
{
	char errbuf[PCAP_ERRBUF_SIZE];

	pcap_t* adhandle = pcap_open(m_AdapterName, // 设备名
		MAX_SNAP_LEN,	// 可处理的数据包长度
		PCAP_OPENFLAG_PROMISCUOUS, // 混杂模式
		1000,  // 读取超时时间
		NULL, // 远程机器验证
		errbuf); // 错误缓冲池

	if (adhandle == NULL)
	{
		printf("\nError: cannot open the adapter!");
		pcap_freealldevs(m_alldevs);
	}
	return adhandle;
}

/**
* 获取一个可用的网络适配器对应的 ip 地址(本机 ip 地址)及其掩码.
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

pcap_if_t* NetworkAdapter::getAllDevsPointer()
{
	return m_alldevs;
}

pcap_if_t* NetworkAdapter::getDevPointer()
{
	return m_d;
}

char* NetworkAdapter::getAdapterName()
{
	return m_AdapterName;
}

BOOLEAN NetworkAdapter::GetSelfMac(PUCHAR MacAddr)
{
	LPADAPTER lpAdapter = PacketOpenAdapter(m_AdapterName + 8); // `+8` 跳过"rpcap:\\"
	if (!lpAdapter || (lpAdapter->hFile == INVALID_HANDLE_VALUE))
		return FALSE;

	PPACKET_OID_DATA pOidData = (PPACKET_OID_DATA)malloc(
		sizeof(PACKET_OID_DATA) + MAC_LEN); // 查看结构体定义，结合MAC地址的长度，便可知道'+6'的含义
	if (pOidData == NULL)
	{
		PacketCloseAdapter(lpAdapter);
		return FALSE;
	}

	// Retrieve the adapter MAC querying the NIC driver
	pOidData->Oid = OID_802_3_CURRENT_ADDRESS; // 获取 MAC 地址
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
		// 第一条 dwPhysAddrLen 不为 0 的项就是默认网关
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
	return ret; // return (-1) if failed
}

/**
* 获取适配器参数:
* 1. 将`m_d`定位到一个可用的网络适配器
* 2. 获取适配器数量`m_devnum`
* 3. 获取`m_d`指向的适配器的名字, 结果保存在`m_AdapterName`
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
			// 描述信息中含有关键字 "Microsoft" 的适配器即为可用的适配器
			if (StrStrIA(d->description, "Microsoft"))
			{
				m_d = d; // 1. 定位到可用的网络适配器
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
		m_devnum = count; // 2. 网络适配器数量
		strcpy(m_AdapterName, m_d->name); // 3. 网络适配器名字
	}
}
