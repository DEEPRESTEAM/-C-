#include "workClient.h"
// 客户端登录
struct sockaddr_in clientSin;

// 写一个链表将你说过的话存储起来
// 创建链表
chatCon_t *creatHead()
{
    chatCon_t *head = malloc(sizeof(chatCon_t));
    if (head == NULL)
    {
        ERRLOG("cread head error");
        return NULL;
    }
    head->next = NULL;
}

// 单向循环链表插入
void insertNode(chatCon_t *head, char *data)
{
    if (head == NULL)
    {
        ERRLOG("链表不存在");
    }
    // 这里使用尾插法
    chatCon_t *temp = head;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }
    chatCon_t *newNode = malloc(sizeof(chatCon_t));
    if (newNode == NULL)
    {
        ERRLOG("malloc error");
    }
    strcpy(newNode->buf, data);

    // 连接到节点中
    temp->next = newNode;
}

// 遍历链表所有的内容
void prinChatCon(chatCon_t *head)
{
    if (head == NULL || head->next == NULL)
    {
        printf("链表为空或链表不存在\n");
    }
    chatCon_t *temp = head->next;
    while (temp != NULL)
    {
        printf("%s\n", temp->buf);
        temp = temp->next;
    }
}

// 界面初始化
void uiInit(char *fileName)
{
    struct stat st;
    int uiFd = open(fileName, O_RDONLY);
    if (uiFd == -1)
    {
        printf("文件不存在或打开错误\n");
        return;
    }
    stat(fileName, &st);
    int fileLen = st.st_size;
    char *bufUi = malloc(fileLen + 1);
    memset(bufUi, 0, st.st_size + 1);
    read(uiFd, bufUi, fileLen);
    close(uiFd);
    printf("%s\n", bufUi);
    free(bufUi);
}

int clientInit()
{

    clientSin.sin_family = AF_INET;
    clientSin.sin_port = htons(2042);

    // 将计算机小段字节序转换为网络方面的大端字节序
    if (inet_pton(AF_INET, "127.0.0.1", &clientSin.sin_addr.s_addr) == -1)
    {
        ERRLOG("转换失败");
    }

    // 创建TCP套接字
    int clientFd = 0;
    if ((clientFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ERRLOG("socket error");
    }

    // 用于连接服务端
    if (connect(clientFd, (struct sockaddr *)&clientSin, sizeof(clientSin)) == -1)
    {
        ERRLOG("connect error");
    }
    return clientFd;
}

// 客户端注册
void registe(int fd)
{
    user_t user;
    state_t state;
    while (1)
    {

        printf("输入你的账户:");
        scanf("%s", user.uid);
        if (!strcmp(user.uid, "return"))
        {
            printf("退出登录\n");
            break;
        }
        printf("输入你的密码:");
        scanf("%s", user.pwd);
        printf("输入你的电话:");
        scanf("%lu", &user.phone);
        printf("输入你的姓名或昵称:");
        scanf("%s", user.name);
        // 首先先把状态设置为注册状态
        state.flag = REGIST;
        memcpy(state.data, &user, sizeof(user_t));
        char buf[1024] = {0};
        write(fd, &state, sizeof(state_t));
        // 接收服务器验证后的数据
        read(fd, buf, sizeof(buf));

        if (!strcmp(buf, "exit"))
        {
            printf("uid已存在请重新输入\n");
            memset(buf, 0, sizeof(buf));
            continue;
        }
        if (!strcmp(buf, "OK"))
        {
            printf("注册成功，请登录\n");
            memset(buf, 0, sizeof(buf));
            break;
        }
    }
}

// 登录,应该返回当前用户的信息，以便后续业务的处理
user_t *Login(int fd)
{
    user_t *user = malloc(sizeof(user_t));
    state_t state;
    int flag = 0;
    while (1)
    {
        printf("输入uid:");
        scanf("%s", user->uid);
        if (!strcmp(user->uid, "return"))
        {
            printf("退出登录\n");
            return NULL;
        }
        printf("输入密码:");
        scanf("%s", user->pwd);
        state.flag = LOGIN;
        memcpy(state.data, user, sizeof(user_t));
        write(fd, &state, sizeof(state_t));
        char buf[20] = {0};
        read(fd, buf, sizeof(buf));
        if (!strcmp(buf, "OK"))
        {
            printf("登录成功，正在进入主界面\n");
            return user;
        }
        else if (!strcmp(buf, "PWDERROR"))
        {
            printf("密码错误 , 请重新输入\n");
            flag++;
            if (flag == 5)
            {
                printf("密码输入次数过多，已强制退出\n");
                return NULL;
            }

            continue;
        }
        else if (!strcmp(buf, "ALREADYLOGIN"))
        {
            printf("用户已经登录，请勿重新登录\n");
            return NULL;
        }
        else
        {
            printf("用户不存在，请注册用户\n");
            return NULL;
        }
    }
}

// 用户忘记密码
void forgetPwd(int fd)
{
    user_t user;
    state_t state;
    printf("——————————————————忘记密码界面————————————————\n");
    printf("输入你的uid:");
    scanf("%s", user.uid);
    printf("输入你的电话号码:");
    scanf("%ld", &user.phone);
    // 将状态置为忘记密码
    state.flag = FORGETPWD;
    memcpy(state.data, &user, sizeof(user_t));
    write(fd, &state, sizeof(state));
    char buf[20] = {0};
    read(fd, buf, sizeof(buf));
    if (!strcmp(buf, "OK"))
    {
        // 说明服务器验证成功，发送需要修改的密码
        while (1)
        {
            printf("填写新密码:");
            scanf("%s", user.pwd);
            printf("再次输入新密码:");
            memset(buf, 0, sizeof(buf));
            scanf("%s", buf);
            if (strcmp(user.pwd, buf))
            {
                printf("两次密码输入不一致请重新输入\n");
                continue;
            }
            else
            {
                // 设置状态码为更新数据，然后发送到服务器中进行更新数据
                state.flag = UPDATAUSER;
                memcpy(state.data, &user, sizeof(user_t));
                write(fd, &state, sizeof(state));
                char buf[20] = {0};
                read(fd, buf, sizeof(buf));
                if (!strcmp(buf, "UPOK"))
                {
                    printf("更新成功\n");
                    return;
                }
            }
        }
    }
    else
    {
        printf("身份认证不足\n");
    }
}

/*******************************************************二级控制台对应的函数*******************************************************************/

// 用户退出到主界面意味着用户退出登录，这时需要发送退出消息给服务器，
int quitLogin(int fd, user_t *user)
{
    state_t state;
    // 重置状态码为退出
    state.flag = QUIT;
    // 将用户信息传递给服务端便于更改状态
    memcpy(state.data, user, sizeof(user_t));
    write(fd, &state, sizeof(state));
    char buf[20];
    read(fd, buf, sizeof(buf));
    printf("%s\n", buf);
    if (!strcmp(buf, "OK"))
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

// 用户群发消息，需要用信号量控制子进程的进行
int allChat(int fd, user_t *user, chatCon_t *head)
{
    // 当用户选择此功能先发状态发给服务器，让其转发
    state_t state;
    pid_t pid;
    pid = fork();
    if (pid > 0)
    {
        printf("你已进入世界聊天，可以开始聊天了(输入 return 退出世界聊天)\n");
        while (1)
        {
            char buf[1024];
            scanf("%s", state.data);
            getchar();
            sprintf(buf, "世界聊天(聊天内容)：%s", state.data);
            //nsertNode(head, buf);
            if (!strcmp(state.data, "return"))
            {
                kill(pid, SIGKILL);
                return 1;
            }
            // 将状态置为群聊状态
            state.flag = CHATALL;
            memcpy(state.userData, user, sizeof(user_t));
            write(fd, &state, sizeof(state));
            memset(&state, 0, sizeof(state));
        }
    }
    else if (pid == 0)
    {
        while (1)
        {
            state_t state;
            read(fd, &state, sizeof(state));
            if (state.flag == CHATALL)
            {
                printf("%s\n", state.data);
            }
        }
    }
}

// 打印所有用户信息
void userList(int fd, user_t *user)
{
    // 首先得获取所有好友的信息并打印出来
    // 给服务器发送消息，获取服务器中的好友列表
    // 每次都要获取服务器的信息
    state_t state;
    state.flag = USERLIST;
    memcpy(state.userData, user, sizeof(user_t));
    printf("%s\n", user->uid);
    write(fd, &state, sizeof(state));
    // 首先发消息给服务端，让服务端发送用户列表数据，也可以自己输入uid进行添加好友
    // memset(&stat , 0 , sizeof(state));
    read(fd, &state, sizeof(state));
    // 这时我们读取了数据，可以将其打印出来
    dbData_t *data = (dbData_t *)state.data;
    // 这时我们可以获取用户名与uid了
    char buf[1024];
    for (int i = 1; i <= data->row; i++)
    {
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
    }
}

// 添加好友业务 。 需要一个好友表，数据结构为 本人 本人uid 本人 name 好友uid 好友name 好友 phone 好友fd 好友在线状态 ， 在线聊天需要做改正
void addFriend(int fd, user_t *user)
{
    // 显示所有的用户信息
    user_t friend;
    state_t state;
    pid_t pid = 0;
    printf("\n");
    while (1)
    {
        printf("请输入你所需要添加好友的uid");
        scanf("%s", friend.uid);
        // 将状态置为添加好友
        state.flag = ADDFRIEND;
        memcpy(&state.data, &friend, sizeof(user_t));
        memcpy(&state.userData, user, sizeof(user_t));
        // 将数据发送到服务器中
        write(fd, &state, sizeof(state));
        // 接收服务器的发送的消息
        read(fd, &state, sizeof(state));
        if (state.flag == OK)
        {
            printf("发送消息成功\n");
            break;
        }
        else if (state.flag == ERROR)
        {
            printf("输入的用户不存在\n");
            continue;
        }
    }
}

// 好友请求业务，用户添加好友的消息会发送到此处来接收
void addFriendRequest(int fd, user_t *user)
{
    // 这里需要创建子进程程用于服务端的接收
    pid_t pid = 0;
    state_t state;
    user_t friend;
    int num = 0;
    read(fd, &state, sizeof(state));
    user_t *user1 = (user_t *)state.userData;
    pid = fork();
    if (pid > 0)
    {
        while (1)
        {
            printf("输入你的选项，输入 -1 退出\n");
            scanf("%d", &num);
            if (num == 0)
            {
                // 不同意加好友
                continue;
            }
            else if (num == 1)
            {

                // 将自己的信息存入在data中
                memcpy(&state.data, user, sizeof(user));
                // 将状态设置为添加好友请求
                state.flag = ADDFRIENDREQUEST;
                write(fd, &state, sizeof(state));
                memset(&state, 0, sizeof(state));
                read(fd, &state, sizeof(state));
                if (state.flag == OK)
                {
                    printf("添加好友成功\n");
                    sleep(1);
                    kill(pid, SIGKILL);
                    break;
                }
                else if (state.flag == ERROR)
                {
                    printf("好友已存在\n");
                    kill(pid, SIGKILL);
                    break;
                }
            }
            else if (num == -1)
            {
                // 退出这个模块
                kill(pid, SIGKILL);
                return;
            }
        }
    }
    else if (pid == 0)
    {
        char buf[1024];
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
    }
}

// 好友私聊
void friendChat(int fd, user_t *user, chatCon_t *head)
{
    user_t us;
    state_t state;
    pid_t pid;
    pid = fork();
    if (pid > 0)
    {
        while (1)
        {
            printf("输入你想私聊的uid：(输入return退出)\n");
            scanf("%s", us.uid);
            if (!strcmp(us.uid, "return"))
            {
                kill(pid, SIGKILL);
                return;
            }
            else
            {
                state.flag = PRIVATECHAT;
                while (1)
                {
                    char buf[1024];
                    scanf("%s", buf);
                    char buf1[1024];
                    sprintf(buf1, "好友私聊:你对%s说了：%s", us.uid, buf);
                    //insertNode(head, buf1);
                    if (!strcmp(buf, "return"))
                    {
                        kill(pid, SIGKILL);
                        return;
                    }

                    char buf2[1024];
                    sprintf(buf2, "%s 对你说：%s", user->uid, buf);

                    memcpy(&state.data, buf1, sizeof(buf2));
                    memcpy(&state.friendData, &us, sizeof(user_t));
                    memcpy(&state.userData, user, sizeof(user_t));

                    write(fd, &state, sizeof(state));
                }
            }
        }
    }
    else if (pid == 0)
    {
        while (1)
        {
            printf("---------------接收到的消息--------------\n");
            state_t state;
            read(fd, &state, sizeof(state));
            printf("%s\n", state.data);
        }
    }
}

void deleteFriend(int fd, user_t *user)
{
    // 删除好友
    state_t state;
    user_t us;
    printf("请输入需要删除的好友uid");
    scanf("%s", us.uid);
    // 将状态置为删除好友态
    state.flag = DELETEFRIEND;
    memcpy(state.userData, user, sizeof(user_t));
    memcpy(state.friendData, &us, sizeof(user_t));
    // 将其发送到服务端
    write(fd, &state, sizeof(state));
    memset(&state, 0, sizeof(state));
    read(fd, &state, sizeof(state));
    if (state.flag == OK)
    {
        printf("删除成功\n");
    }
    else
    {
        printf("删除失败 , 服务端错误\n");
    }
}

void showFriend(int fd, user_t *user)
{
    state_t state;
    printf("===========显示好友列表===========\n");
    // 将标识符设置为好友列表
    state.flag = USERLIST;
    memcpy(&state.userData, user, sizeof(user_t));
    write(fd, &state, sizeof(state));
    read(fd, &state, sizeof(state));
    // 这时我们读取了数据，可以将其打印出来
    dbData_t *data = (dbData_t *)state.data;
    // 这时我们可以获取用户名与uid了
    char buf[1024];
    for (int i = 1; i <= data->row; i++)
    {
        read(fd, buf, sizeof(buf));
        printf("%s\n", buf);
    }
}

// 建立群聊
void setFriendChat(int fd, user_t *user)
{
    // 首先输入你想要一起聊天的好友uid()

    // 输入你好友的uid建立群聊
    // 首先你需要创建几个人的群聊
    user_t us;
    state_t state;
    char buf[1024];
    pid_t pid;
    printf("输入你要拉的好友uid（以/分割以;结尾）\n");
    scanf("%s", buf);
    state.flag = SETFRIENDCHAT;
    memcpy(&state.friendData, buf, sizeof(buf));
    memcpy(&state.userData, user, sizeof(user_t));
    strcpy(state.temp , "OK");
    printf("------------输入你的内容(return 退出)---------\n");
    pid = fork();
        if (pid > 0)
        {
            while (1)
            {
                char str[1024];
                scanf("%s", str);
                if (!strcmp(str, "return"))
                {
                    state.flag = SETFRIENDCHAT;
                    strcpy(state.temp, "QUITE");
                    write(fd, &state, sizeof(state));
                    kill(pid, SIGKILL);
                    return;
                }
                memcpy(&state.data, str, sizeof(str));
                write(fd, &state, sizeof(state));
            }
        }
        else
        {
            while (1)
            {
                printf("---------------接收到的消息--------------\n");
                state_t state;
                read(fd, &state, sizeof(state));
                printf("%s\n", state.data);
            }
        }
  
}
// 群聊请求
void joinFriendChat(int fd , user_t * user)
{
    //从服务器获取群聊的fd
    state_t state;
    pid_t pid;
    printf("------------输入你的内容(return 退出)---------\n");
    pid = fork();
    if (pid > 0)
    {
        while (1)
        {
            char str[1024];
            scanf("%s", str);
            if (!strcmp(str, "return"))
            {
                kill(pid, SIGKILL);
                return;
            }
            state.flag = ADDFRIENDCHAT;
            memcpy(&state.data, str, sizeof(str));
            write(fd, &state, sizeof(state));
        }
        
    }else{
        while (1)
        {

            printf("---------------接收到的消息--------------\n");
            state_t state;
            read(fd, &state, sizeof(state));
            printf("%s\n", state.data);
        }
      
    }
    
}

// memset(sql, 0, 256);
// sprintf(sql, "SELECT name FROM %s WHERE name='%s'", data.msg.my_name, group_name);
// printf("%s\n", sql);
// if (SQLITE_OK != sqlite3_get_table(data.my_db, sql, &pazResult, &pnRow, &pnColumn, NULL))
// {
//     printf("error = %s\n", sqlite3_errmsg(data.my_db));
//     my_error(&senddata, data.acceptfd);
//     sqlite3_free_table(pazResult);
//     return -1;
// }
// if (pnRow < 1)
// {
//     sprintf(senddata.MsgBuff, "你还没有%s的好友", group_name);
//     write_senddata(&senddata, 'B', "ok", NULL, NULL);
//     send(data.acceptfd, &senddata, sizeof(maseg_t), 0);
// }
// else
// {
//     memset(sql, 0, 256);
//     sprintf(sql, "INSERT INTO %s_gp VALUES(1,'%s')", data.msg.name, group_name);
//     if (SQLITE_OK != sqlite3_exec(data.my_db, sql, NULL, NULL, NULL))

//     {
//         printf("INSTER INTO：sqlite3_exec error:%s\n", sqlite3_errmsg(data.my_db));
//         my_error(&senddata, data.acceptfd);
//         sqlite3_free_table(pazResult);
//         return -1;
//     }
// }
// memset(group_name, 0, sizeof(group_name));
// i_name = 0;