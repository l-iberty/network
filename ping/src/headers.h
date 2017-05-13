#ifndef HEADERS_H
#define HEADERS_H

#define ICMP_ECHO       8
#define ICMP_ECHOREPLY  0

struct ICMP_HDR {
    unsigned char icmp_type;
    unsigned char icmp_code;
    unsigned short icmp_cksum;
    unsigned short icmp_id;
    unsigned short icmp_seq;
};

struct IP_HDR {
    unsigned char ip_verlen;            // 4位版本号, 4位首部长度
    unsigned char ip_tos;               // 8位服务类型TOS
    unsigned short ip_totallen;         // 16位数据报总长度（字节）
    unsigned short ip_id;               // 16位标识
    unsigned short ip_frag_and_offset;  // 3位标志位, 13位偏移
    unsigned char ip_ttl;               // 8位生存时间 TTL
    unsigned char ip_proto;             // 8位协议 (TCP, UDP 或其他)
    unsigned short ip_checksum;         // 16位IP首部校验和
    unsigned int ip_srcip;              // 32位源IP地址
    unsigned int ip_dstip;              // 32位目的IP地址
};

#endif //HEADERS_H
