#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include "headers.h"

#define SRC_ADDR "127.0.0.1"
#define DST_ADDR "127.0.0.1"
#define SRC_PORT 10000
#define DST_PORT 9999
#define BUFSIZE 60


int close_socket(int sock) {
    if (close(sock)) {
        perror("close");
        return 1;
    }
    return 0;
}

/**
 * 计算校验和
 * @param p    数据缓存区
 * @param len  数据长度(Byte)
 * @return     校验和
 */
u16 cksum(u16 *p, int len) {
    int cksum = 0;
    u16 answer = 0;

    // 以16bits为单位累加
    while (len > 1) {
        u16 t = *p;
        cksum += *p++;
        len -= 2;
    }
    // 如果数据的字节数为奇数, 将最后一个字节视为16bits的高8bits, 低8bits补0, 继续累加
    if (len == 1) {
        answer = *(u8 *) p;
        cksum += answer;
    }
    // cksum是32bits的int, 而校验和需为16bits, 需将cksum的高16bits加到低16bits上
    cksum = (cksum >> 16) + (cksum & 0xffff);
    // 按位求反
    return (~(u16) cksum);
}

/**
 * 对16bits整数v进行大-小端序的转换
 */
u16 transval_16(u16 v) {
    return ((v >> 8) | (v << 8));
}


void print_bytes(u8 *buf, int len) {
    printf("\n");
    for (int i = 0; i < len; i++) {
        printf("%.2x", buf[i]);
        if ((i + 1) % 2 == 0) printf(" ");
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

int main() {
    int sockfd, opt;
    struct sockaddr_in target_addr;
    struct ip_hdr ip_hdr;
    struct tcp_hdr tcp_hdr;
    struct psd_hdr psd_hdr;
    u8 send_pkt[BUFSIZE];

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close_socket(sockfd);
        return 1;
    }

    ip_hdr.ip_verlen = ((0x4 << 4) | (sizeof(ip_hdr) / sizeof(int)));
    ip_hdr.ip_tos = 0x00;
    ip_hdr.ip_totallen = transval_16(sizeof(ip_hdr) + sizeof(tcp_hdr));
    ip_hdr.ip_id = 0xAAAA;
    ip_hdr.ip_frag_and_offset = transval_16(0x4000);
    ip_hdr.ip_ttl = 64;
    ip_hdr.ip_proto = IPPROTO_TCP;
    ip_hdr.ip_checksum = 0;
    ip_hdr.ip_srcip = inet_addr(SRC_ADDR);
    ip_hdr.ip_dstip = inet_addr(DST_ADDR);
    ip_hdr.ip_checksum = transval_16(cksum((u16 *) &ip_hdr, sizeof(ip_hdr)));

    psd_hdr.saddr = inet_addr(SRC_ADDR);
    psd_hdr.daddr = inet_addr(DST_ADDR);
    psd_hdr.mbz = 0;
    psd_hdr.proto = IPPROTO_TCP;
    psd_hdr.len = sizeof(tcp_hdr);

    tcp_hdr.tcp_sport = transval_16(SRC_PORT);
    tcp_hdr.tcp_dport = transval_16(DST_PORT);
    tcp_hdr.tcp_seq = 0x78563412;
    tcp_hdr.tcp_ack = 0;
    tcp_hdr.tcp_lenres = (u8) ((0xa << 4) | 0x0);
    tcp_hdr.tcp_flag = 2; // SYN = 1
    tcp_hdr.tcp_win = transval_16(0x156);
    tcp_hdr.tcp_checksum = 0;
    tcp_hdr.tcp_urp = 0;

    size_t buflen = sizeof(psd_hdr) + sizeof(tcp_hdr);
    u8 *buf = (u8 *) malloc(buflen);
    if (buf == NULL) {
        perror("malloc");
        return 1;
    }
    memcpy(buf, &psd_hdr, sizeof(psd_hdr));
    memcpy(buf + sizeof(psd_hdr), &tcp_hdr, sizeof(tcp_hdr));
    tcp_hdr.tcp_checksum = transval_16(cksum((u16 *) buf, buflen));
    free(buf);

    //memcpy(send_pkt, &ip_hdr, sizeof(ip_hdr));
    memcpy(send_pkt, &tcp_hdr, sizeof(tcp_hdr));

    int send_len=sizeof(ip_hdr) + sizeof(tcp_hdr);
    print_bytes(send_pkt, send_len);

    bzero(&target_addr, sizeof(target_addr));
    target_addr.sin_family = AF_INET;
    target_addr.sin_addr.s_addr = inet_addr(DST_ADDR);
    target_addr.sin_port = htons(DST_PORT);

    if (sendto(sockfd, send_pkt, send_len, 0,
               (struct sockaddr *) &target_addr, sizeof(target_addr)) < 0) {
        perror("sendto");
        close_socket(sockfd);
        return 1;
    }

    int sockfd_recv = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd_recv < 0) {
        perror("socket");
        return 1;
    }

    if (setsockopt(sockfd_recv, IPPROTO_IP, IP_HDRINCL, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close_socket(sockfd_recv);
        return 1;
    }
    ssize_t bytes_recv;
    u8 recv_buf[BUFSIZE];
    socklen_t target_len = sizeof(target_addr);
    bytes_recv = recvfrom(sockfd_recv, recv_buf, BUFSIZE, 0,
                          (struct sockaddr *) &target_addr, &target_len);
    if (bytes_recv < 0) {
        perror("recvfrom");
        close_socket(sockfd_recv);
        return 1;
    }

    struct tcp_hdr *p_tcp_hdr = (struct tcp_hdr *) (recv_buf + sizeof(ip_hdr));
    print_bytes((recv_buf + sizeof(ip_hdr)), sizeof(tcp_hdr));

    close_socket(sockfd);
    close_socket(sockfd_recv);

    /*u16 buf[] = {
            // psd_hdr
            0x3dd5, 0xab19, 0xc0a8, 0x0a65, 0x0006, 0x0020,
            // tcp_hdr
            0x0050, 0xb59c, 0xd484, 0x1b59, 0x9f45, 0xeb18,
            0x8010, 0x03ab, 0x0000, 0x0000,
            // data
            0x0101, 0x080a, 0x7f95, 0xe6d6, 0x0003, 0x2406
    };

    printf("%.4x\n", cksum(buf, sizeof(buf) * sizeof(u8)));*/

    //print_bytes((u8 *) &ip_hdr, sizeof(ip_hdr));

    return 0;
}
