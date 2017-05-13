/**
 * 本服务端能并发处理多个客户端连接.
 * 如果有多个分组到达, accept 会按照 FIFO 原则从队列里
 * 取出分组, 随后服务端会处理 accept 返回的分组. 此时队
 * 列里还有可供获取的分组, 这会导致 epoll_wait 返回, 然
 * 后再次调用 accept 获取分组并做处理.
 */
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUF_SIZE 1024
#define LISTENQ 5
#define SERV_PORT 9999
#define MAX_EVENTS 10

int setnonblocking(int sock) {
    int opts;

    // 获取sock的文件状态标志
    opts = fcntl(sock, F_GETFL);
    if (opts == -1) {
        perror("fcntl(sock, F_GETFL)");
        return 1;
    }

    // 设置非阻塞I/O
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) == -1) {
        perror("fcntl(sock,F_SETFL,opts)");
        return 1;
    }
    return 0;
}

int close_socket(int sock) {
    if (close(sock)) {
        perror("close");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int main() {
    struct epoll_event ev, events[MAX_EVENTS];
    int listen_sock, conn_sock, sockfd, nfds, epollfd;
    struct sockaddr_in addr, cli_addr;
    socklen_t cli_size;
    int conn_num = 0; // 连接数量

    ssize_t readret, writeret;
    char recvbuf[BUF_SIZE];
    char sendbuf[BUF_SIZE];


    /* Code to set up listening socket, 'listen_sock',
       (socket(), bind(), listen()) omitted */

    printf("------ Server ------\n");

    // 创建监听套接字
    if ((listen_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    // socket 地址结构
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERV_PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 绑定监听套接字
    if (bind(listen_sock, (struct sockaddr *) &addr, sizeof(addr))) {
        perror("bind");
        close_socket(listen_sock);
        return EXIT_FAILURE;
    }

    // 监听
    if (listen(listen_sock, LISTENQ)) {
        perror("listen");
        close_socket(listen_sock);
        return EXIT_FAILURE;
    }

    // 创建 epoll
    epollfd = epoll_create(256);
    if (epollfd == -1) {
        perror("epoll_create");
        close_socket(listen_sock);
        return EXIT_FAILURE;
    }

    // 将 listen_sock 与读事件关联, 并注册到 epollfd.
    // 当有客户机发起连接时, 相应的分组达到连接队列
    // (《Unix网络编程》中有详述), 则 listen_sock 可读
    // 而不会阻塞, 从而导致 epoll_wait 返回.
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        close_socket(listen_sock);
        return EXIT_FAILURE;
    }

    // main loop
    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            close_socket(listen_sock);
            return EXIT_FAILURE;
        }

        // 遍历处理所有事件
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_sock) {
                // 有分组到达, accept 返回该分组
                cli_size = sizeof(cli_addr);
                conn_sock = accept(listen_sock,
                                   (struct sockaddr *) &cli_addr, &cli_size);

                if (conn_sock == -1) {
                    // 无法获取分组
                    perror("accept");
                    close_socket(listen_sock);
                    return EXIT_FAILURE;
                }

                // 连接套接字设置为非阻塞模式
                setnonblocking(conn_sock);

                printf("Received a connection from %s\n",
                       inet_ntoa(cli_addr.sin_addr));
                printf(">> conn_num = %d\n", ++conn_num);

                // 对于 accept 返回的客户端连接套接字 'conn_sock', 在 epoll 模型中
                // 不再使用 recv 函数读取数据, 而是将其注册到 epollfd, 使得在下一轮循
                // 环中作为读事件被处理. (debug时发现, conn_sock 和读事件中的 sockfd
                // 等值)
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                              &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    close_socket(listen_sock);
                    close_socket(conn_sock);
                    return EXIT_FAILURE;
                }
            } else if (events[i].events & EPOLLIN) {
                // 读事件

                // 判断 file descriptor 是否有效
                if ((sockfd = events[i].data.fd) == -1)
                    continue;

                while ((readret = read(sockfd, recvbuf, BUF_SIZE)) > 0) {
                    recvbuf[readret] = '\0';
                    printf("\nReceived data from client [%s]: %s",
                           inet_ntoa(cli_addr.sin_addr), recvbuf);
                }

                if ((readret == -1) && (errno != EAGAIN)) {
                    perror("read");
                    close_socket(listen_sock);
                    close_socket(sockfd);
                    return EXIT_FAILURE;
                }

                // 对客户端连接套接字 conn_sock 的读取完毕, 之后需要向该连接套接字
                // 写入, 以此向客户端发送消息. epoll 模型中不使用 send, 而是向 epollfd
                // 注册写操作事件. (sockfd 和 conn_sock 等值)
                ev.events = EPOLLOUT | EPOLLET;
                ev.data.fd = sockfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
                    perror("epoll_ctl: MOD, EPOLLOUT | EPOLLET");
                    close_socket(listen_sock);
                    close_socket(sockfd);
                    return EXIT_FAILURE;
                }
            } else if (events[i].events & EPOLLOUT) {
                // 写事件
                if ((sockfd = events[i].data.fd) == -1)
                    continue;

                sprintf(sendbuf, "Message from server: conn_num = %d", conn_num);
                writeret = write(sockfd, sendbuf, strlen(sendbuf));
                if (writeret == -1) {
                    if (errno == EPIPE) {
                        printf("Connection closed at [%s]\n",
                               inet_ntoa(cli_addr.sin_addr));
                        conn_num--;
                        continue;
                    } else {
                        perror("wirte");
                        close_socket(listen_sock);
                        close_socket(sockfd);
                        return EXIT_FAILURE;
                    }
                }

                // 向连接套接字写入成功, 重新注册读事件, 等待客户端发送消息
                // 通过debug证实, 对于同一个客户端连接, 之后如果客户端再次
                // 发送消息, 在读事件中处理的 file descriptor 与此处的
                // sockfd 等值.
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = sockfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
                    perror("epoll_ctl: MOD, EPOLLIN | EPOLLET");
                    close_socket(listen_sock);
                    close_socket(sockfd);
                    return EXIT_FAILURE;
                }
            }
        }
    }
}
