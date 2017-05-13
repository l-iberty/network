#ifndef HEADERS_H
#define HEADERS_H

#define HOSTPORT 10000
#define HOSTPORT_TXT "10000"
#define HOSTNAME "127.0.0.1"

struct tcp_hdr {
    unsigned short tcp_sport; // 源端口号
    unsigned short tcp_dport; // 目的端口号
    unsigned int tcp_seq; // 序号
    unsigned int tcp_ack; // 确认号
    unsigned char tcp_lenres; // 4 bits 的数据偏移和 4 bits 的保留字段
    unsigned char tcp_flag; // 标志
    unsigned short tcp_win; // 窗口长度
    unsigned short tcp_checksum; // 校验和
    unsigned short tcp_urp; // 紧急指针
};

struct ip_hdr {
    unsigned char ip_verlen; // 4位版本号, 4位首部长度
    unsigned char ip_tos; // 8位服务类型TOS
    unsigned short ip_totallen; // 16位数据报总长度（字节）
    unsigned short ip_id; // 16位标识
    unsigned short ip_frag_and_offset;  // 3位标志位, 13位偏移
    unsigned char ip_ttl; // 8位生存时间 TTL
    unsigned char ip_proto; // 8位协议 (TCP, UDP 或其他)
    unsigned short ip_checksum;  // 16位IP首部校验和
    unsigned int ip_srcip; // 32位源IP地址
    unsigned int ip_dstip; // 32位目的IP地址
};

#endif // HEADERS_H