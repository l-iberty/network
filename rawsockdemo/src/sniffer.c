#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "headers.h"

#define BUF_SIZE 1024

int close_socket(int sock) {
    if (close(sock)) {
        perror("close");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void get_ip(int ip_val, char *ip_buf) {
    int v[4];
    char s[4];
    v[0] = (ip_val) & 0xff;
    v[1] = (ip_val >> 8) & 0xff;
    v[2] = (ip_val >> 16) & 0xff;
    v[3] = (ip_val >> 24) & 0xff;

    for (int i = 0; i < 4; i++) {
        sprintf(s, "%d", v[i]);
        strcat(ip_buf, s);
        strcat(ip_buf, ".");
    }
    ip_buf[strlen(ip_buf) - 1] = '\0';
}

unsigned short get_port(unsigned short raw_port) {
    unsigned short ret_port;

    ret_port = (raw_port >> 8) & 0xff;
    ret_port |= (raw_port & 0xff) << 8;

    return ret_port;
}

unsigned short get_checksum(unsigned short raw_checksum) {
    unsigned short ret_checksum;

    ret_checksum = (raw_checksum >> 8) & 0xff;
    ret_checksum |= (raw_checksum & 0xff) << 8;

    return ret_checksum;
}

void print_flags(unsigned char flag) {
    printf("[ URG:%d ", (flag >> 5) & 0x01);
    printf("ACK:%d ", (flag >> 4) & 0x01);
    printf("PSH:%d ", (flag >> 3) & 0x01);
    printf("RST:%d ", (flag >> 2) & 0x01);
    printf("SYN:%d ", (flag >> 1) & 0x01);
    printf("FIN:%d ]\n", (flag) & 0x01);

}

void print_bytes(unsigned char *buf, ssize_t len) {
    for (int i = 0; i < len; i++) {
        printf("%.2x", buf[i]);
        if ((i + 1) % 2 == 0) printf(" ");
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

void print_iphdrinfo(struct ip_hdr *iphdr) {
    char srcip[64] = {0};
    char dstip[64] = {0};
    get_ip(iphdr->ip_srcip, srcip);
    get_ip(iphdr->ip_dstip, dstip);

    printf("IP: %s -> %s\n", srcip, dstip);
    switch (iphdr->ip_proto) {
        case IPPROTO_TCP:
            printf("Protocol: TCP\n");
            break;
        case IPPROTO_UDP:
            printf("Protocol: UDP\n");
            break;
        default:
            printf("Protocol: Others\n");
    }
}

void print_tcphdrinfo(struct tcp_hdr *tcphdr) {
    printf("Port: %d -> %d\n",
           get_port(tcphdr->tcp_sport),
           get_port(tcphdr->tcp_dport));
    printf("Flags: ");
    print_flags(tcphdr->tcp_flag);
    printf("Check sum: %d\n", get_checksum(tcphdr->tcp_checksum));
}

int main(int argc, char **argv) {
    unsigned char buf[BUF_SIZE] = {0};
    struct sockaddr_in addr, cli_addr;
    socklen_t cli_size;
    int raw_sockfd;
    ssize_t bytes_recv;
    struct ip_hdr *iphdr;
    struct tcp_hdr *tcphdr;

    // 创建原始套接字
    // 协议类型为 TCP, 所以后续抓取的数据包的 IP Header 的协议类型字段都是 TCP
    raw_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (raw_sockfd < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // 开启 IP_HDRINCL 选项, 以获取 IP Header.
    // When set to TRUE, indicates the application provides the IP header.
    // Applies only to SOCK_RAW sockets.
    const int flag = 1;
    if (setsockopt(raw_sockfd, IPPROTO_IP, IP_HDRINCL,
                   &flag, sizeof(flag)) < 0) {
        perror("setsockopt");
        close_socket(raw_sockfd);
        return EXIT_FAILURE;
    }

    printf("Ready to accept...\n");
    while (1) {
        printf("-----------------------------------------\n");

        cli_size = sizeof(cli_addr);
        bytes_recv = recvfrom(raw_sockfd, buf, sizeof(buf), 0,
                              (struct sockaddr *) &cli_addr, &cli_size);
        if (bytes_recv < 0) {
            perror("recvfrom");
            close_socket(raw_sockfd);
            return EXIT_FAILURE;
        }

        printf("\nBytes received:\n");
        print_bytes(buf, bytes_recv);

        printf("\nIP Header:\n");
        print_bytes(buf, sizeof(struct ip_hdr));
        iphdr = (struct ip_hdr *) buf;
        print_iphdrinfo(iphdr);

        printf("\nTCP Header:\n");
        print_bytes(buf + sizeof(struct ip_hdr), sizeof(struct tcp_hdr));
        tcphdr = (struct tcp_hdr *) (buf + sizeof(struct ip_hdr));
        print_tcphdrinfo(tcphdr);
    }
}
