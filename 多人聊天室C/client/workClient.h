#ifndef _WORKCLIENT_H_
#define _WORKCLIENT_H_

#include <head.h>
#define MAX 18
#define CHAR_SIZE 1024
// 用于存放用户个人信息，以及客户端信息，会发送个服务端不是链表
// 先处理好各个功能的识别号
#define LOGIN 0        // 登录
#define REGIST 1       // 注册
#define CHATALL 2      // 群聊（所有聊天）
#define FORGETPWD 3    // 忘记密码
#define UPDATAUSER 4   // 修改用户信息
#define ADDFRIEND 5    // 添加好友
#define USERLIST 6   // 获取用户列表
#define DELETEFRIEND 7 // 删除好友
#define PRIVATECHAT 8  // 私聊好友
#define ADDFRIENDREQUEST 9    // 添加好友请求
#define APRIVATECHAT 10       // 私聊好友中
#define SETFRIENDCHAT 11       // 好友群聊
#define ADDFRIENDCHAT 12       // 加入好友群聊
#define QUIT -1        // 退出
#define OK 100
#define ERROR -100

// 存放用户的信息
typedef struct user
{
    char uid[MAX];           // 用户账号
    char pwd[MAX];           // 用户密码
    char name[MAX];          // 用户昵称
    unsigned long int phone; // 用户电话号码
} user_t;

typedef struct st
{
    int flag;
    char data[1024]; // 可作为用户聊天内容，也可作为传入验证信息
    char userData[1024];
    char friendData[1024];
    char temp[1024];
} state_t;

// 聊天内容结构体
typedef struct chatContent
{
    char con[1024];
    struct sockaddr_in clientSin;
} chatContent_t;

// 写一个历史记录链表，将你说过的语句全部存储起来
typedef struct chatCon
{
    struct chatCon *next;
    char buf[1024];
} chatCon_t;

// 用于存储查找的内容
typedef struct dbData
{
    int row;
    int col;
    char **result;
} dbData_t;

// 写一个链表将你说过的话存储起来
// 创建链表
chatCon_t *creatHead();

// 单向循环链表插入
void insertNode(chatCon_t *head, char *data);

// 遍历链表所有的内容
void prinChatCon(chatCon_t *head);

// 客户端初始化，返回初始化完成后创建的套接字
int clientInit();

// 界面初始化
void uiInit(char *fileName);

// 主功能集成
void mastControl(int fd, chatCon_t *head);

// 用户注册
void registe(int fd);

// 用户登录
user_t *Login(int fd);

// 用户忘记密码
void forgetPwd(int fd);

// 二级控制台
void secondController(int fd, user_t *user, chatCon_t *head);

// 用户群发消息，需要用信号量控制子进程的进行
int allChat(int fd, user_t *user, chatCon_t *head);

// 用户退出到主界面意味着用户退出登录，这时需要发送退出消息给服务器，
int quitLogin(int fd, user_t *user);

// 三级控制台
void friendModle(int fd, user_t *user , chatCon_t * head);

// 添加好友业务 。 需要一个好友表，数据结构为 本人 本人uid 本人 name 好友uid 好友name 好友 phone 好友fd 好友在线状态 ， 在线聊天需要做改正
void addFriend(int fd, user_t *user);

//打印所有用户信息，以便于添加好友
void userList(int fd, user_t *user);

// 好友请求业务，用户添加好友的消息会发送到此处来接收
void addFriendRequest(int fd, user_t * user);

// 好友私聊
void friendChat(int fd, user_t *user , chatCon_t * head);

void deleteFriend(int fd, user_t *user);

void showFriend(int fd, user_t *user);

// 建立群聊
void setFriendChat(int fd, user_t *user);

// 群聊请求
void joinFriendChat(int fd, user_t *user);

#endif
