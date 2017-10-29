/**
 * 实现ping命令的核心功能--ICMP报文的收发与统计.
 * 旨在实现核心部分, 因此未实现域名解析(仅接收IP地址作为输入), 未实现
 * 超时控制(如果ping不可达的主机,只会发送一个报文,接收报文的函数会一直
 * 不响应), 未实现诸多复杂的参数选项.
 */

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

#define BUF_SIZE 64

// 全局变量
struct timeval g_tvsend;
struct timeval g_tvrecv;
int g_sockfd;
int g_sendnum = 0, g_recvnum = 0;

// 函数原型
unsigned short cksum(unsigned short *p, int len);
void set_pkt(struct ICMP_HDR *p_icmphdr, unsigned short seq);
void send_pkt(int sockfd, char *hostname, unsigned char *buf, size_t len);
void recv_pkt(int sockfd);
int parse(unsigned char *buf, ssize_t len, struct sockaddr_in addr);
void tv_sub(struct timeval *out, struct timeval *in);
void statistics();
unsigned short transval_16(unsigned short v);
void print_icmphdr(struct ICMP_HDR *p_icmphdr, size_t len);

// main entry
int main(int argc, char **argv) {
    struct ICMP_HDR icmp_hdr;
    unsigned char sendbuf[BUF_SIZE];

    if (argc != 2) {
        printf("usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    g_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (g_sockfd < 0) {
        perror("socket");
        return 1;
    }

    bzero(sendbuf, BUF_SIZE);
    int seq = 1;
    signal(SIGINT, statistics);
    printf("PING %s %d bytes of data\n", argv[1], BUF_SIZE);
    for (;;) {
        set_pkt(&icmp_hdr, seq++);
        memcpy(sendbuf, &icmp_hdr, sizeof(icmp_hdr));
        send_pkt(g_sockfd, argv[1], sendbuf, BUF_SIZE);
        recv_pkt(g_sockfd);
        sleep(1); // 发报间隔: 1s
    }
}

/**
 * 计算校验和
 * @param p    数据缓存区
 * @param len  数据长度(Byte)
 * @return     校验和
 */
unsigned short cksum(unsigned short *p, int len) {
    int cksum = 0;
    unsigned short answer = 0;

    // 以16bits为单位累加
    while (len > 1) {
        unsigned short t = *p;
        cksum += *p++;
        len -= 2;
    }
    // 如果数据的字节数为奇数, 将最后一个字节视为16bits的高8bits, 低8bits补0, 继续累加
    if (len == 1) {
        answer = *(unsigned char *) p;
        cksum += answer;
    }
    // cksum是32bits的int, 而校验和需为16bits, 需将cksum的高16bits加到低16bits上
    cksum = (cksum >> 16) + (cksum & 0xffff);
    // 按位求反
    return (~(unsigned short) cksum);
}

/**
 * 填充ICMP Header
 * @param p_icmphdr _Out_
 * @param seq _In_ 指定的序号
 */
void set_pkt(struct ICMP_HDR *p_icmphdr, unsigned short seq) {
    p_icmphdr->icmp_type = ICMP_ECHO;
    p_icmphdr->icmp_code = 0;
    p_icmphdr->icmp_cksum = 0;
    // 为便于后续parse()的判断操作,icmp_id不使用小端序填充
    p_icmphdr->icmp_id = (unsigned short) getpid();
    // icmp_seq使用小端序填充
    p_icmphdr->icmp_seq = transval_16(seq);
    /**
     * 校验和的填充必须使用网络顺序，即大端序
     */
    unsigned short t = cksum((unsigned short *) p_icmphdr,
                             sizeof(struct ICMP_HDR));
    p_icmphdr->icmp_cksum = (t);
}

void send_pkt(int sockfd, char *hostname, unsigned char *buf, size_t len) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(hostname);
    ssize_t sendlen;

    sendlen = sendto(sockfd, buf, len, 0,
                     (struct sockaddr *) &addr, sizeof(addr));
    gettimeofday(&g_tvsend, NULL); // 记录发报时间
    if (sendlen < 0) {
        perror("sendto");
    }
    g_sendnum++; // 发报数加1
}

void recv_pkt(int sockfd) {
    struct sockaddr_in addr;
    socklen_t addr_len;
    unsigned char recvbuf[BUF_SIZE];
    ssize_t recvlen;

    addr_len = sizeof(addr);
    recvlen = recvfrom(sockfd, recvbuf, BUF_SIZE, 0,
                       (struct sockaddr *) &addr, &addr_len);
    gettimeofday(&g_tvrecv, NULL); // 记录收报时间
    if (recvlen < 0) {
        perror("recvfrom");
    }
    g_recvnum++; // 收报数加1
    parse(recvbuf, recvlen, addr); // 解析收到的报文
}

/**
 * 报文解析
 */
int parse(unsigned char *buf, ssize_t len, struct sockaddr_in addr) {
    struct IP_HDR *p_iphdr;
    int ip_hl, icmp_hl;
    struct ICMP_HDR *p_icmphdr;
    double rtt;

    p_iphdr = (struct IP_HDR *) buf;
    ip_hl = (p_iphdr->ip_verlen & 0x0f) << 2; // 计算IP Header的长度
    p_icmphdr = (struct ICMP_HDR *) (buf + ip_hl); // 越过IP Header, 解析ICMP Header
    if ((icmp_hl = len - ip_hl) < 8) {
        // 若 (数据报长度-IP首部长度) < ICMP首部长度8, 则发生了错误
        printf("icmp_hl (%d) < 8\n", icmp_hl);
        return 0;
    }
    //print_icmphdr(p_icmphdr, sizeof(struct ICMP_HDR));

    // 判断收到的ICMP报文是否是回复我发送的ICMP报文
    if (p_icmphdr->icmp_type == ICMP_ECHOREPLY &&
        p_icmphdr->icmp_id == getpid()) {
        tv_sub(&g_tvrecv, &g_tvsend);
        // 以ms为单位计算往返时延rtt
        rtt = g_tvrecv.tv_sec * 1000 + (double) g_tvrecv.tv_usec / 1000;
        printf("%d bytes from %s: icmp_seq=%d ttl=%d rtt=%.3f ms\n",
               len,
               inet_ntoa(addr.sin_addr),
               transval_16(p_icmphdr->icmp_seq),
               p_iphdr->ip_ttl,
               rtt);
    }
    return 0;
}

/**
 * 计算时间差(out-in). 代码取自《Unix网络编程第一卷》第25章
 * @param out _In_Out_
 * @param in _In_
 */
void tv_sub(struct timeval *out, struct timeval *in) {
    if ((out->tv_usec -= in->tv_usec) < 0) {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

/**
 * 接收到Ctrl-C信号后,统计并显示
 */
void statistics() {
    printf("\n--- ping statistics ---\n");
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n",
           g_sendnum, g_recvnum,
           (g_sendnum - g_recvnum) / g_sendnum * 100);
    close(g_sockfd);
    exit(1);
}

/**
 * 对16bits整数v进行大-小端序的转换
 */
unsigned short transval_16(unsigned short v) {
    unsigned short ret = 0;
    ret = (v >> 8) | (v << 8);
    return ret;
}

/**
 * 打印ICMP Header的所有字节
 */
void print_icmphdr(struct ICMP_HDR *p_icmphdr, size_t len) {
    unsigned char *ptr;
    printf("\n---- ICMP Header ----\n");
    ptr = (unsigned char *) p_icmphdr;
    for (int i = 0; i < len; i++) {
        printf("%.2x ", *ptr++);
        if ((i + 1) % 4 == 0) printf("\n");
    }
    printf("---- END ----\n");
}
