#include "work.h"
struct sockaddr_in clientSin;

/**
 * 仅限于服务器业务不要参杂其他业务
 */

int serviceInit()
{
    serviceSin.sin_family = AF_INET;
    serviceSin.sin_port = htons(2042);
    if (inet_pton(AF_INET, "127.0.0.1", &serviceSin.sin_addr.s_addr) == -1)
    {
        ERRLOG("转换失败\n");
    }
    int serviceFd = 0;
    // 创建TCP套接字
    if ((serviceFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ERRLOG("sock TCP error");
    }
    /* 设置套接字允许IP地址和端口被重新设置 */
    /* 允许IP地址和端口被重新设置 */
    int options = 1;
    if (setsockopt(serviceFd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options)) == -1)
    {
        ERRLOG("set socket error");
    }
    // 绑定
    if (bind(serviceFd, (struct sockaddr *)&serviceSin, sizeof(serviceSin)) == -1)
    {
        ERRLOG("bind error");
    }
    // 给套接字设置监听
    if (listen(serviceFd, 1024) == -1)
    {
        ERRLOG("listen error");
    }

    return serviceFd;
}

// 主题结构采用IO复用
void struMain(int serviceFd)
{
    struct epoll_event event;
    struct epoll_event events[512];
    int epfd = 0;
    int ret = 0;
    int count = 0;
    /* 创建一个epoll实例 */
    epfd = epoll_create(512);
    if (epfd == -1)
    {
        perror("epoll_create");
        return;
    }

    /* 将文件描述符listenfd及其读事件添加到epoll实例中 */
    event.events = EPOLLIN;
    event.data.fd = serviceFd;
    // 将监听套接字和事件结构体天际到epoll的监管集合中，EPOLL_CTL_ADD是添加的意思，对应的还有删除
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, serviceFd, &event);
    if (ret == -1)
    {
        perror("epoll_ctl->EPOLL_CTL_ADD");
        return;
    }
    // 数据库初始化
    sqlite3 *db = initsqlLite();
    while (1)
    {
        /* 检测是否有资源准备就绪，准备就绪返回就绪的IO资源数 */
        // events里面存放了发生可读事件的epoll_event结构元素，所以遍历时只需要遍历count次就可以了
        count = epoll_wait(epfd, events, 512, 1500000);
        printf("有%d个元素有可读事件\n", count);
        if (count == -1)
        {
            perror("epoll_wait");
            return;
        }
        else if (count == 0)
        {
            fprintf(stderr, "epoll_wait timeout ...\n");
            continue;
        }
        for (int i = 0; i < count; i++)
        {
            int fd = 0;
            fd = events[i].data.fd; /* 得到准备就绪集合中的文件描述符 */
            if (events[i].events == EPOLLIN)
            {
                /* 读事件 */
                if (fd == serviceFd) /* 监听套接字资源准备就绪：说明有新的客户端发起连接请求 */
                {
                    /* 4. 服务器等待监听客户端的连接请求并建立连接 */
                    socklen_t len = sizeof(clientSin);
                    int connectFd = 0;
                    connectFd = accept(serviceFd, (struct sockaddr *)&clientSin, &len);
                    if (connectFd == -1)
                    {
                        perror("accept");
                        return;
                    }
                    printf("connfd = %d client connect success\n", connectFd);

                    /* 上树：有新的客户端连接成功,将新的通信套接字文件描述符及其事件添加到epoll实例中 */
                    event.events = EPOLLIN;
                    event.data.fd = connectFd;
                    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, connectFd, &event);
                    if (ret == -1)
                    {
                        perror("epoll_ctl->EPOLL_CTL_ADD");
                        return;
                    }
                }
                else
                {

                    state_t state;
                    ret = read(fd, &state, sizeof(state_t));
                    user_t *user = (user_t *)state.data;
                    user_t *userData = (user_t *)state.user;
                    char buf[1024] = {0};
                    dbData_t data;
                    switch (state.flag)
                    {
                    case REGIST:
                        // 首先查询表中是否存在此用户，如果存在提醒用户uid已存在请重新输入
                        sprintf(buf, "select * from user where uid = '%s'", user->uid);
                        data = findData(db, buf);
                        if (data.row >= 1)
                        {

                            write(fd, "exit", sizeof("exit"));
                            break;
                        }
                        else
                        {
                            // 账户不存在，需要注册账户
                            memset(buf, 0, sizeof(buf));
                            sprintf(buf, "INSERT INTO user(uid , pwd , name , fd , phone , online) VALUES ('%s', '%s', '%s', %d , %ld , 0);", user->uid, user->pwd, user->name, fd, user->phone);
                            addData(db, buf);
                            write(fd, "OK", sizeof("OK"));
                            break;
                        }
                        memset(buf, 0, sizeof(buf));
                        break;
                    case LOGIN:
                        // 首先查询用户账户是否存在
                        sprintf(buf, "select * from user where uid = '%s'", user->uid);
                        data = findData(db, buf);
                        if (data.row >= 1)
                        {
                            sprintf(buf, "select * from user where uid = '%s' and online != %d", user->uid, 1);
                            dbData_t data1 = findData(db, buf);
                            if (data1.row > 0)
                            {
                                int postion = getFinResult(data1.row, data1.col, 3);

                                if (!strcmp(data1.result[postion], user->pwd))
                                {
                                    // 如果正确发送ok给客户端，表示可以登录
                                    write(fd, "OK", sizeof("OK"));
                                    // 把当前用的登录状态户置为已上线
                                    sprintf(buf, "update user set fd = '%d' where uid = '%s'", fd, user->uid);
                                    changeData(db, buf);

                                    sprintf(buf, "update user set online = '%d' where uid = '%s'", 1, user->uid);
                                    changeData(db, buf);

                                    sprintf(buf, "update friend set userFd = %d where userid = '%s'", fd, user->uid);
                                    changeData(db, buf);

                                    sprintf(buf, "update friend set FriendFd = %d where friendUid = '%s'", fd, user->uid);
                                    changeData(db, buf);
                                }
                                else
                                {
                                    // 如果错误提醒客户端密码错误
                                    write(fd, "PWDERROR", sizeof("PWDERROR"));
                                }
                            }
                            else
                            {
                                // 用户已经登录无法重复登录
                                write(fd, "ALREADYLOGIN", sizeof("ALREADYLOGIN"));
                            }
                        }
                        else
                        {
                            // 用户没找到，提醒客户端用户不存在
                            write(fd, "USERNOTEXIST", sizeof("USERNOTEXIST"));
                        }
                        break;
                    case FORGETPWD:
                        // 首先查询该用户是否符合省份验证
                        sprintf(buf, "select * from user where uid = '%s'and phone = %ld ", user->uid, user->phone);
                        data = findData(db, buf);
                        if (data.row > 0)
                        {
                            // 说明权限足够发送可以修改的标志
                            write(fd, "OK", sizeof("OK"));
                        }
                        else
                        {
                            write(fd, "NOTOK", sizeof("NOTOK"));
                        }
                        break;
                    case UPDATAUSER: // 忘记密码后续，更新密码
                        // 只有通过了身份验证才能到达这一步，进行密码修改
                        sprintf(buf, "update user set pwd = '%s' where uid = '%s'", user->pwd, user->uid);
                        changeData(db, buf);
                        // 发个消息给客户端修改成功
                        write(fd, "UPOK", sizeof("UPOK"));
                        break;
                    case CHATALL:
                        // 首先查询所有已经在线的用户
                        sprintf(buf, "select fd from user where fd != %d and online = 1", fd);
                        data = findData(db, buf);
                        // 根据客户端发来的用户信息查询用户的姓名
                        sprintf(buf, "select name from user where uid = '%s'", userData->uid);
                        dbData_t data1 = findData(db, buf);
                        sprintf(buf, "客户端用户%s发送一条消息:%s", data1.result[1], state.data);
                        memset(&state, 0, sizeof(state));
                        memcpy(&state.data, buf, sizeof(buf));
                        // 将状态设置为群聊
                        state.flag = CHATALL;
                        for (int i = 1; i < data.row + 1; i++)
                        {
                            write(atoi(data.result[i]), &state, sizeof(state));
                            printf("fd = %d  发送了 = %s\n", fd, state.data);
                        }
                        break;
                    case USERLIST:
                        // 首先将表中除本身以外的所有的fd查询出来
                        sprintf(buf, "select friendUid from friend where userid = '%s'", userData->uid);
                        data = findData(db, buf);
                        // 将服务器查询的数据量转发到客户端上
                        memcpy(state.data, &data, sizeof(data));
                        write(fd, &state, sizeof(state));
                        for (int i = 1; i <= data.row; i++)
                        {
                            // 首先将其拼接
                            sprintf(buf, "uid:%s  ", data.result[i]);
                            // 然后转发给客户端
                            write(fd, buf, sizeof(buf));
                        }
                        break;
                    case ADDFRIEND:
                        // 添加好友 , 且不能自己添加好友
                        sprintf(buf, "select fd from user where uid = '%s'and uid != '%s' ", user->uid, userData->uid);
                        // 这个添加好友只是发送消息给在线的用户，还需要一个处理添加好友的业务
                        data = findData(db, buf);
                        if (data.row > 0)
                        {
                            // 说明找到此用户了
                            sprintf(buf, "用户%s希望添加你为好友,是否同意(1|0)", userData->uid);
                            // 将验证消息发送给当前用户看用户是否同意
                            if (atoi(data.result[1]) > 0)
                            {
                                // 发送自己的信息给当前用户
                                write(atoi(data.result[1]), &state, sizeof(state));
                                // 发个发送成功的消息给当前用户
                                write(atoi(data.result[1]), buf, sizeof(buf));

                                memset(&state, 0, sizeof(state));
                                state.flag = OK;
                                write(fd, &state, sizeof(state));
                            }
                            else
                            {
                                printf("fd = %d\n", atoi(data.result[1]));
                                // 给当前用户发个错误标志位
                                memset(&state, 0, sizeof(state));
                                state.flag = ERROR;
                                write(fd, &state, sizeof(state));
                            }
                        }
                        else
                        {
                            // 给当前用户发个错误标志位
                            memset(&state, 0, sizeof(state));
                            state.flag = ERROR;
                            write(fd, &state, sizeof(state));
                        }
                        break;
                    case ADDFRIENDREQUEST:
                        // 将好友信息与自己的信息存入到数据库中给客户端发送个OK即可
                        // 先将用户FD查询出来，插入表中
                        sprintf(buf , "select * from friend where userid = '%s' and friendUid = '%s'" , user->uid , userData->uid);
                        data = findData(db , buf);
                        if (data.row == 0)
                        {

                            sprintf(buf, "select fd from user where uid = '%s'", userData->uid);
                            data = findData(db, buf);

                            // char bufUser[1024];
                            sprintf(buf, "select fd from user where uid = '%s'", user->uid);
                            dbData_t dataUser = findData(db, buf);

                            sprintf(buf, "insert into friend values ('%s' , %d , '%s' , '%d')", userData->uid, atoi(data.result[1]), user->uid, atoi(dataUser.result[1]));
                            addData(db, buf) != -1;
                            char buf1[1024];
                            sprintf(buf1, "insert into friend values ('%s' , %d , '%s' , '%d')", user->uid, atoi(dataUser.result[1]), userData->uid, atoi(data.result[1]));

                            if (addData(db, buf1) != -1)
                            {
                                memset(&state, 0, sizeof(state));
                                state.flag = OK;
                                write(fd, &state, sizeof(state));
                            }
                        }else
                        {
                            // 给当前用户发个错误标志位
                            memset(&state, 0, sizeof(state));
                            state.flag = ERROR;
                            write(fd, &state, sizeof(state));
                        }
                        break;
                    case DELETEFRIEND:
                    {
                        sprintf(buf, "delete from friend where userid = '%s' and friendUid = '%s'", userData->uid, ((user_t *)state.friendData)->uid);
                        int ret1 = deleteData(db, buf);
                        char buf1[1024];
                        sprintf(buf1, "delete from friend where userid = '%s' and friendUid = '%s'", ((user_t *)state.friendData)->uid, userData->uid);
                        int ret2 = deleteData(db, buf1);
                        if (ret1 == 0 && ret2 == 0)
                        {
                            memset(&state, 0, sizeof(state));
                            state.flag = OK;
                            write(fd, &state, sizeof(state));
                        }
                        else
                        {
                            memset(&state, 0, sizeof(state));
                            state.flag = ERROR;
                            write(fd, &state, sizeof(state));
                        }

                        break;
                    }
                    case PRIVATECHAT:
                        sprintf(buf, "select friendFd from friend where friendUid = '%s' and userid = '%s'", ((user_t *)state.friendData)->uid, userData->uid);
                        data = findData(db, buf);
                        if (data.row > 0)
                        {
                            printf("%s\n", state.data);
                            state.flag = OK;
                            printf("fd = %d\n", atoi(data.result[1]));
                            if (atoi(data.result[1]) != 0)
                            {
                                write(atoi(data.result[1]), &state, sizeof(state));
                            }
                        }
                        else
                        {
                            // 说明没找到好友
                            state.flag = ERROR;
                            strcpy(state.data, "用户不存在");
                            write(fd, &state, sizeof(state));
                        }
                        break;
                    case SETFRIENDCHAT: 
                        if (!strcmp(state.temp, "OK"))
                        {

                            //这里还要对是否是好友做处理
                            sprintf(buf, "select * from friendChat where member = '%s'", userData->uid);
                            dbData_t data2 = findData(db, buf);
                            if (data2.row == 0)
                            {
                                sprintf(buf, "insert into friendChat values ('%s' , %d) ", userData->uid, fd);
                                addData(db, buf);
                            }
                            char group_name[1024] = {0};
                            int i_name = 0;
                            for (int i = 0; i < strlen(state.friendData); i++)
                            {

                                if (state.friendData[i] != '/' && state.friendData[i] != ';')
                                {
                                    printf("%c ", state.friendData[i]);
                                    group_name[i_name++] = state.friendData[i];
                                    continue;
                                }
                                sprintf(buf , "select * from friend where userid = '%s' and friendUid = '%s'" , userData->uid , group_name);
                                printf("%s\n", buf);
                                data = findData(db , buf);
                                if (data.row > 0)
                                {

                                    //说明是好友
                                    sprintf(buf, "select * from friendChat where member = '%s'", group_name);
                                    memset(&data, 0, sizeof(data));
                                    data = findData(db, buf);
                                    printf("row = %d\n", data.row);
                                    if (data.row == 0)
                                    {
                                        // 说明，没有加入到数据库中
                                        //  将好友信息加入到群聊中
                                        memset(buf , 0 , sizeof(0));
                                        sprintf(buf, "select friendFd from friend where userid = '%s' and friendUid = '%s'", userData->uid, group_name);
                                        printf("%s\n", buf);
                                        //memset(&data, 0, sizeof(data));
                                        data = findData(db, buf);
                                        printf("row1 = %d " , data.row);
                                        sprintf(buf, "insert into friendChat values('%s' , %d)", group_name, atoi(data.result[1]));
                                        addData(db, buf);
                                    }
                                    // 万一好友上线我需要更改他们的状态
                                    // 首先查询
                                    sprintf(buf, "select fd from user where uid = '%s'", group_name);
                                    memset(&data, 0, sizeof(data));
                                    data = findData(db, buf);
                                    memset(buf, 0, sizeof(buf));
                                    sprintf(buf, "update friendChat set meFd = %d where member = '%s'", atoi(data.result[1]), group_name);
                                    changeData(db, buf);

                                    // 需要从群聊中取出所有人除了自己的fd发送消息
                                    memset(buf, 0, sizeof(buf));
                                    sprintf(buf, "select meFd from friendChat where member = '%s'", group_name);
                                    printf("%s\n", buf);
                                    memset(&data, 0, sizeof(data));
                                    data = findData(db, buf);

                                    if (atoi(data.result[1]) != 0)
                                    {
                                        printf("fd = %d\n", atoi(data.result[1]));
                                        write(atoi(data.result[1]), &state, sizeof(state));
                                    }
                                    memset(&group_name, 0, sizeof(group_name));
                                    i_name = 0;
                                }else{
                                    //说明不是好友
                                    //跳过不做处理即可
                                    memset(&group_name, 0, sizeof(group_name));
                                    i_name = 0;
                                }
                            }
                        }
                        else if (!strcmp(state.temp, "QUITE"))
                        {
                            // 销毁群聊，删除群聊表中的所有内容
                            sprintf(buf, "delete from friendChat");
                            deleteData(db, buf);
                        }
                        break;
                    case ADDFRIENDCHAT:
                        sprintf(buf , "select meFd from friendChat where meFd != %d" , fd);
                        data = findData(db , buf);
                        if (data.row > 0)
                        {
                            for (int i = 1; i <= data.row; i++)
                            {
                                write(atoi(data.result[i]) , &state , sizeof(state));
                            }
                        }else{
                            memset(&state, 0, sizeof(state));
                            state.flag = ERROR;
                            write(fd, &state, sizeof(state));
                        }
                        
                        break;
                    case QUIT:
                        // 如果是退出状态会到这一步，这时候更改当前用户的状态并返回一个成功的状态码， 因为没有退出客户端所以不需要更改fd
                        sprintf(buf, "update user set online = %d where uid = '%s'", 0, user->uid);
                        // 退出的话还要修改friend的fd
                        changeData(db, buf);
                        sprintf(buf, "update friend set userFd = %d where userid = '%s'", 0, user->uid);
                        changeData(db, buf);

                        sprintf(buf, "update friend set FriendFd = %d where friendUid = '%s'", 0, user->uid);
                        changeData(db, buf);
                        // 发个消息给客户端修改成功
                        write(fd, "OK", sizeof("OK"));
                        break;
                    default:
                        break;
                    }

                    if (ret == -1)
                    {
                        perror("read");
                        return;
                    }
                    else if (ret == 0)
                    {
                        /*说明对端(客户端)退出：完成下树，将退出的文件描述符fd及其事件从epoll实例中删除 */
                        // 首先将当前用户的在数据库中的fd与在线状态清0
                        sprintf(buf, "update user set online = %d where fd = %d", 0, fd);
                        changeData(db, buf);

                        sprintf(buf, "update user set fd = '%d' where fd = '%d'", 0, fd);
                        changeData(db, buf);

                        changeData(db, buf);
                        sprintf(buf, "update friend set userFd = %d where userid = '%s'", 0, user->uid);
                        changeData(db, buf);

                        sprintf(buf, "update friend set FriendFd = %d where friendUid = '%s'", 0, user->uid);
                        changeData(db, buf);

                        event.data.fd = fd;
                        ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event);
                        if (ret == -1)
                        {
                            perror("epoll_ctl->EPOLL_CTL_DEL");
                            return;
                        }
                        close(fd);
                        break;
                    }
                }
            }
        }
    }
}
