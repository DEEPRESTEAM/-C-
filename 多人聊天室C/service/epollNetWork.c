#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/epoll.h>

int main()
{
    int listenfd;
    int connfd;
    int ret;
    int optval;
    int count;
    struct sockaddr_in srvaddr;
    struct sockaddr_in cltaddr;
    socklen_t addrlen = sizeof(cltaddr);
    int fd;
    int epfd;
    char buf[1024];
    int i;
    struct epoll_event event;
    struct epoll_event events[512];

    /* 1. 创建TCP流式套接字文件 */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket");
        return -1;
    }

    /* 设置套接字允许IP地址和端口被重新设置 */
    optval = 1; /* 允许IP地址和端口被重新设置 */
    ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret == -1)
    {
        perror("setsockopt");
        return -1;
    }

    /* 2. 设置服务器主机的IP地址和端口号 */
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(8888);
    srvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(listenfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr));
    if (ret == -1)
    {
        perror("bind");
        return -1;
    }

    /* 3. 启动服务器监听客户端的连接请求 */
    ret = listen(listenfd, 1024);
    if (ret == -1)
    {
        perror("listen");
        return -1;
    }
    printf("listenfd = %d server init success\n", listenfd);

    /* 创建一个epoll实例 */
    epfd = epoll_create(512);
    if (epfd == -1)
    {
        perror("epoll_create");
        return -1;
    }

    /* 上树：将文件描述符listenfd及其读事件添加到epoll实例中 */
    event.events = EPOLLIN;
    event.data.fd = listenfd;
    // 将监听套接字和事件结构体天际到epoll的监管集合中，EPOLL_CTL_ADD是添加的意思，对应的还有删除
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &event);
    if (ret == -1)
    {
        perror("epoll_ctl->EPOLL_CTL_ADD");
        return -1;
    }

    while (1)
    {
        /* 检测是否有资源准备就绪，准备就绪返回就绪的IO资源数 */
        // events里面存放了发生可读事件的epoll_event结构元素，所以遍历时只需要遍历count次就可以了
        count = epoll_wait(epfd, events, 512, 1500000);
        printf("有%d个元素有可读事件\n", count);
        if (count == -1)
        {
            perror("epoll_wait");
            return -1;
        }
        else if (count == 0)
        {
            fprintf(stderr, "epoll_wait timeout ...\n");
            continue;
        }
        for (i = 0; i < count; i++)
        {
            fd = events[i].data.fd; /* 得到准备就绪集合中的文件描述符 */
            if (events[i].events == EPOLLIN)
            {
                /* 读事件 */
                if (fd == listenfd) /* 监听套接字资源准备就绪：说明有新的客户端发起连接请求 */
                {
                    /* 4. 服务器等待监听客户端的连接请求并建立连接 */
                    connfd = accept(listenfd, (struct sockaddr *)&cltaddr, &addrlen);
                    if (connfd == -1)
                    {
                        perror("accept");
                        return -1;
                    }
                    printf("connfd = %d client connect success\n", connfd);

                    /* 上树：有新的客户端连接成功,将新的通信套接字文件描述符及其事件添加到epoll实例中 */
                    event.events = EPOLLIN;
                    event.data.fd = connfd;
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &event);
                    if (ret == -1)
                    {
                        perror("epoll_ctl->EPOLL_CTL_ADD");
                        return -1;
                    }
                }
                else /* 通信套接字资源准备就绪：说明有已经连接的客户端发送数据请求 */
                {

                    // 最基础的怎样来判断服务器选择功能，客户端传来什么消息来使服务器做出不同的业务选择

                    // 在客户端与服务器连接里可以做一些相应的业务

                    // 最普通的群聊

                    // 怎样添加好友？添加好友需要另外一个人的同意，然后通过链表的形式 存储好友的通讯消息？？这里链表里该存储什么信息才能用于好友通信？

                    // 通过连接的方式查看好友是否在线，通过在线的方式，可以在线聊天，也可以发送离线消息，好友上号就能收到发送的消息，然后进行回复
                    // 怎么处理离线消息，首先做一个缓存，将离线消息存储在缓存中，一旦服务端监听到好友上号，就将缓存的数据发送到好友的客户端消息

                    // 附加功能，看是否能实现文件传输功能，对好友进行文件发送，或者群发，不过得通过管理员的审查（可以发送音乐，图片，文档，ppt等）

                    // 数据结构得分清楚，首先在服务端有个给离线消息缓存的数据结构，里面包含哪些内容  现阶段：（好友的套接字（怎么获取是个问题） ， 缓存的内容）

                    /* 接收客户端的数据请求 */
                    memset(buf, 0, sizeof(buf));
                    ret = read(fd, buf, sizeof(buf));
                    if (ret == -1)
                    {
                        perror("read");
                        return -1;
                    }
                    else if (ret == 0)
                    {
                        /*说明对端(客户端)退出：完成下树，将退出的文件描述符fd及其事件从epoll实例中删除 */
                        event.data.fd = fd;
                        ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
                        if (ret == -1)
                        {
                            perror("epoll_ctl->EPOLL_CTL_DEL");
                            return -1;
                        }
                        close(fd);
                        break;
                    }
                    else
                    {
                        printf("%s\n", "收到了来自客户端的消息");
                        printf("%s\n", buf);
                        // /* 调用write数据的回写 */
                        // ret = read(fd, buf, sizeof(buf));
                        // if (ret == -1)
                        // {
                        //     perror("write");
                        //     return -1;
                        // }
                    }
                }
            }
        }
    }
    /*服务器退出：完成下树，将退出的文件描述符listenfd及其事件从epoll实例中删除 */
    event.data.fd = listenfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, listenfd, &event);
    if (ret == -1)
    {
        perror("epoll_ctl->EPOLL_CTL_DEL");
        return -1;
    }

    close(listenfd);
    close(epfd);
    return 0;
}