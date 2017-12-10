#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>

#define PORT 9999
#define BUF_SIZE 256

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        printf("usage: %s <server-ip> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sock;
    struct addrinfo hints;
    char recv_buf[BUF_SIZE];
    char send_buf[BUF_SIZE];
    
    bzero(&hints, sizeof(hints));
    struct addrinfo *servinfo;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo(argv[1], argv[2], &hints, &servinfo) != 0)
    {
        perror("getaddrinfo");
        return EXIT_FAILURE;
    }
    
    if((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        perror("socket");
        return EXIT_FAILURE;
    }
    
    if (connect (sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        perror("connect");
        return EXIT_FAILURE;
    }

    printf("\n输入数据(exit退出): ");
    fgets(send_buf, BUF_SIZE, stdin);

    while (strncmp(send_buf, "exit" ,4)) {
        int bytes_received;
        printf("Sent from client>>: %s", send_buf);

        send(sock, send_buf , strlen(send_buf), 0);

        if((bytes_received = recv(sock, recv_buf, BUF_SIZE, 0)) > 1)
        {
            recv_buf[bytes_received] = '\0';
            printf("Received from server<<: %s", recv_buf);
        }
        printf("\n输入数据(exit退出): ");
        fgets(send_buf, BUF_SIZE, stdin);
    }

    freeaddrinfo(servinfo);
    close(sock);
    return EXIT_SUCCESS;
}
