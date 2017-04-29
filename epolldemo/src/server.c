/******************************************************************************
* server.c                                                                    *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char recvBuf[BUF_SIZE] = { 0 };
    char sendBuf[] = "Default message from server";

    fprintf(stdout, "----- Server -----\n");

    /* 创建监听套接字 */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;		// IPV4
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* 绑定监听套接字*/
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        perror("bind");
        return EXIT_FAILURE;
    }

    /* 监听 */
    if (listen(sock, 5)) // 监听队列的长度为5
    {
        close_socket(sock);
        perror("listen");
        return EXIT_FAILURE;
    }

    /* 循环检测是否有连接请求，若有则创建连接套接字并做相应处理 */
    while (1)
    {
        cli_size = sizeof(cli_addr);
        if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                  &cli_size)) == -1)
        {
            close(sock);
            fprintf(stderr, "Error accepting connection.\n");
            return EXIT_FAILURE;
        }

        readret = 0;

        //从连接套接口接收客户端发送来的数据
	// recv 会一直等待, 直到有数据可读或客户端断开连接, 因此
	// 本服务端不能同时处理一个以上的客户端连接
        while((readret = recv(client_sock, recvBuf, BUF_SIZE, 0)) > 1)
        {
            printf("Received message from client: %s\n", recvBuf);

            //将接收的数据直接发送给客户端，可修改返回的数据
            if (send(client_sock, sendBuf, sizeof(sendBuf), 0) == -1)
            {
                close_socket(client_sock);
                close_socket(sock);
                fprintf(stderr, "Error sending message to client.\n");
                return EXIT_FAILURE;
            }
            memset(recvBuf, 0, BUF_SIZE);
        }

        if (readret == -1)
        {
            close_socket(client_sock);
            close_socket(sock);
            fprintf(stderr, "Error reading from client socket.\n");
            return EXIT_FAILURE;
        }

        if (close_socket(client_sock))
        {
            close_socket(sock);
            fprintf(stderr, "Error closing client socket.\n");
            return EXIT_FAILURE;
        }
	else
	    printf("closed client_sock\n");
    }

    close_socket(sock);

    return EXIT_SUCCESS;
}
