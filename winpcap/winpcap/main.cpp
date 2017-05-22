// 抓取链路层的以太网帧, 同时捕获网络层的IP数据包和传输层的TCP/UDP数据包.
// 整个数据包结构如下:
// 以太网首部-IP首部-TCP/UDP首部-数据

#define _CRT_SECURE_NO_WARNINGS
#include "Pcap.h"

#pragma comment(lib,"ws2_32.lib")

int main(int argc, char **argv)
{
	Pcap pcap = Pcap();

	pcap.print_devs();
	pcap.select_dev();
	pcap.open_adapter();

	if (pcap.open_adapter()) {
		if (pcap.check_datalink()) {
			pcap.set_netmask();
			if (pcap.set_filter())
				pcap.start_cap();
		}
	}
}