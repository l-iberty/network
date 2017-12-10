#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ECHO_PORT 9999
#define BUF_SIZE 256

int close_socket(int sock) {
    if (close(sock)) {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int sock, client_sock;
    ssize_t nr_recv;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char recv_buf[BUF_SIZE] = {0};
    char send_buf[BUF_SIZE] = "Default message from server";

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET; // IPV4
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr))) {
        close_socket(sock);
        perror("bind");
        return EXIT_FAILURE;
    }

    if (listen(sock, 5)) // 监听队列的长度为5
    {
        close_socket(sock);
        perror("listen");
        return EXIT_FAILURE;
    }

    printf("----- Server -----\n");
    printf("Listening at %d ...\n", ECHO_PORT);

    while (1) {
        cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                  &cli_size)) == -1) {
            close(sock);
            perror("accept");
            return EXIT_FAILURE;
        }

        while ((nr_recv = recv(client_sock, recv_buf, BUF_SIZE, 0)) > 1) {
            printf("Received message from client: %s\n", recv_buf);

            if (send(client_sock, recv_buf, sizeof(recv_buf), 0) == -1) {
                close_socket(client_sock);
                close_socket(sock);
                perror("send");
                return EXIT_FAILURE;
            }
            memset(recv_buf, 0, BUF_SIZE);
        }

        if (nr_recv == -1) {
            close_socket(client_sock);
            close_socket(sock);
            perror("recv");
            return EXIT_FAILURE;
        }

        if (close_socket(client_sock)) {
            close_socket(sock);
            fprintf(stderr, "Error closing client socket.\n");
            return EXIT_FAILURE;
        } else {
            printf("closed client_sock\n");
        }
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
