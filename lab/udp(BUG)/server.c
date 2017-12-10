#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERV_PORT 9999
#define BUF_SIZE 256

int close_socket(int sock) {
    if (close(sock)) {
        perror("close");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t addr_len;
    char recv_buf[BUF_SIZE] = {0};
    char send_buf[] = "Default message from server";

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return EXIT_FAILURE;
    }

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
        perror("bind");
        close_socket(sock);
        return EXIT_FAILURE;
    }

    printf("UDP server listening at %d...\n", SERV_PORT);

    while (1) {
        int nr_recv = recvfrom(sock, recv_buf, sizeof(recv_buf), 0,
                               (struct sockaddr *) &cli_addr, &addr_len);
        if (nr_recv < 0) {
            perror("recvfrom");
            close_socket(sock);
            return EXIT_FAILURE;
        }
        recv_buf[nr_recv] = 0;
        printf("data received from client: %s\n", recv_buf);

        if (sendto(sock, send_buf, sizeof(send_buf), 0,
                   (struct sockaddr *) &cli_addr, addr_len) < 0) {
            perror("sendto");
            close_socket(sock);
            return EXIT_FAILURE;
        }
        bzero(recv_buf, sizeof(recv_buf));
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
