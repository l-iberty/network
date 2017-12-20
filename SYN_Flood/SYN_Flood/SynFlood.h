#ifndef SYN_FLOOD
#define SYN_FLOOD

#define USING_OPTIONS

#include "NetworkAdapter.h"

#define WIN_SIZE	63443

class SynFlood :public NetworkAdapter {
public:
	SynFlood();
	~SynFlood();
	void attack(u_int ip, u_short port, u_int t_interval_ms);
private:
	u_short cksum(u_short *p, int len);
	void make_syn_packet(u_char* packet, u_int dst_ip, u_short dst_port);
private:
	u_int m_self_ip;
	u_char m_self_mac[MAC_LEN];
	u_char m_gate_mac[MAC_LEN];
};

#endif // SYN_FLOOD
