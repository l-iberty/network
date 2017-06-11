# A demo using epoll

## 基本框架
* 客户端向服务端发起TCP连接, 服务端收到连接后处理之

## 说明
* `client` 客户端程序
* `server` 最基本的服务端程序，不能同时处理一个以上的连接请求
* `serverepoll` 服务端程序，使用 Linux 下的 epoll 模型，实现对多个连接的管理

## More
* `epoll`的用户手册中提供了一个`epoll`模型的使用框架，通过`man epoll`查看