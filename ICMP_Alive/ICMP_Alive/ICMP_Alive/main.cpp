// ICMP Ö÷»úÌ½²â

#include "IcmpDetect.h"
#include <conio.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Shlwapi.lib")

int main()
{
	u_int net, mask;
	char buf[32];

	printf("Input network address: ");
	scanf("%s", buf);
	net = inet_addr(buf);
	printf("\nInput subnet mask: ");
	scanf("%s", buf);
	mask = inet_addr(buf);

	IcmpDetect* icmpDetect = new IcmpDetect(net, mask);
	icmpDetect->beginDetect();
	delete icmpDetect;

	printf("\n\nAll alive hosts're as above. Press any key to exit...");
	_getch();
}