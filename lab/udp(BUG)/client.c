#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>

#define BUF_SIZE 32

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sock;
    socklen_t serv_len;
    struct sockaddr_in serv_addr;
    char send_buf[BUF_SIZE] = "hello";
    char recv_buf[BUF_SIZE];

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }
    
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons((uint16_t) atoi(argv[1]));

    serv_len = sizeof(serv_addr);
    while (1) {
        if (sendto(sock, send_buf, sizeof(send_buf), 0,
                   (struct sockaddr *) &serv_addr, serv_len) < 0) {
            perror("sendto");
            return EXIT_FAILURE;
        }

        int nr_recv = recvfrom(sock, recv_buf, sizeof(recv_buf), 0,
                               (struct sockaddr *) &serv_addr, &serv_len);
        if (nr_recv < 0) {
            perror("recvfrom");
            return EXIT_FAILURE;
        }
        recv_buf[nr_recv] = 0;
        printf("data received from server: %s\n", recv_buf);
        bzero(recv_buf, sizeof(recv_buf));
    }

    return EXIT_SUCCESS;
}
