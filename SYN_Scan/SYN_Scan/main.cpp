#include "SynScan.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Shlwapi.lib")

int main()
{
	char target_ip[32];
	printf("\nInput IP: ");
	scanf("%s", target_ip);

	SynScan synScan = SynScan(target_ip);
	synScan.beginScan();
}