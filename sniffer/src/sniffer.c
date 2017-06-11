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

u16 get16val(u16 v) {
    u16 ret;

    ret = (v >> 8) & 0xff;
    ret |= (v & 0xff) << 8;

    return ret;
}

void print_flags(u8 flag) {
    printf("[ URG:%d ", (flag >> 5) & 0x01);
    printf("ACK:%d ", (flag >> 4) & 0x01);
    printf("PSH:%d ", (flag >> 3) & 0x01);
    printf("RST:%d ", (flag >> 2) & 0x01);
    printf("SYN:%d ", (flag >> 1) & 0x01);
    printf("FIN:%d ]\n", (flag) & 0x01);

}

void print_bytes(u8 *buf, ssize_t len) {
    for (int i = 0; i < len; i++) {
        printf("%.2x", buf[i]);
        if ((i + 1) % 2 == 0) printf(" ");
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("\n");
}

void print_chars(u8 *buf, ssize_t len) {
    for (int i = 0; i < len; i++) {
	if (buf[i] < 128 && buf[i] > 31) {
            printf("%c", buf[i]);
	}
	else {
	    printf(".");
	}
	if ((i + 1) % 64 == 0) printf("\n");
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

void print_udphdrinfo(struct udp_hdr *udphdr){
    printf("Port: %d -> %d\n",
           get16val(udphdr->udp_sport),
           get16val(udphdr->udp_dport));
    printf("Len: %d\n", get16val(udphdr->udp_len));
    printf("Check sum: %d\n", get16val(udphdr->udp_checksum));
}

void print_tcphdrinfo(struct tcp_hdr *tcphdr) {
    printf("Port: %d -> %d\n",
           get16val(tcphdr->tcp_sport),
           get16val(tcphdr->tcp_dport));
    printf("Flags: ");
    print_flags(tcphdr->tcp_flag);
    printf("Check sum: %d\n", get16val(tcphdr->tcp_checksum));
}

int main(int argc, char **argv) {
    u8 buf[BUF_SIZE] = {0};
    struct sockaddr_in addr, cli_addr;
    socklen_t cli_size;
    int raw_sockfd;
    ssize_t bytes_recv;
    struct ip_hdr *iphdr;
    struct tcp_hdr *tcphdr;
    struct udp_hdr *udphdr;
    int proto = IPPROTO_TCP;	// default TCP

    printf("protocol type: UDP(u) or TCP(t)? ");
    char c;
    scanf("%c", &c);
    if (c == 'u')
	proto = IPPROTO_UDP;
    else if (c == 't')
	proto = IPPROTO_TCP;
    else
	printf("use default: TCP");

    // 创建原始套接字
    raw_sockfd = socket(AF_INET, SOCK_RAW, proto);
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

    int i = 0;
    printf("Ready to accept...\n");
    while (1) {
        printf("\n-----------------< %d >-----------------\n", ++i);

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

	if (iphdr->ip_proto == IPPROTO_TCP) {
	    printf("\nTCP Header:\n");
            print_bytes(buf + sizeof(struct ip_hdr), sizeof(struct tcp_hdr));
            tcphdr = (struct tcp_hdr *) (buf + sizeof(struct ip_hdr));
            print_tcphdrinfo(tcphdr);

            printf("\nData:\n");
	    print_chars(buf + sizeof(struct ip_hdr) + sizeof(struct tcp_hdr),
	    	 bytes_recv - sizeof(struct ip_hdr) - sizeof(struct tcp_hdr));
	}
	else if (iphdr->ip_proto == IPPROTO_UDP) {
	    printf("\nUDP Header:\n");
            print_bytes(buf + sizeof(struct ip_hdr), sizeof(struct udp_hdr));
            udphdr = (struct udp_hdr *) (buf + sizeof(struct ip_hdr));
            print_udphdrinfo(udphdr);

	    printf("\nData:\n");
	    print_chars(buf + sizeof(struct ip_hdr) + sizeof(struct udp_hdr),
	    	get16val(udphdr->udp_len) - sizeof(struct udp_hdr));
	}
	
    }
}
