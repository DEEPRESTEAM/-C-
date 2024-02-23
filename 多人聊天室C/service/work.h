#ifndef _WORK_H_
#define _WORK_H_
#include <head.h>
#include "my_sqlite.h"
#define MAX 18
#define CHAR_SIZE 1024

// 先处理好各个功能的识别号
#define LOGIN 0   // 登录
#define REGIST 1  // 注册
#define CHATALL 2 // 群聊（所有聊天）
#define FORGETPWD 3 //忘记密码
#define UPDATAUSER 4 // 修改用户信息
#define ADDFRIEND 5  // 添加好友
#define USERLIST 6 // 获取用户列表
#define DELETEFRIEND 7 // 删除好友
#define PRIVATECHAT 8  // 私聊好友
#define ADDFRIENDREQUEST 9 // 添加好友请求
#define APRIVATECHAT 10     // 私聊好友中
#define SETFRIENDCHAT 11
#define ADDFRIENDCHAT 12 // 加入好友群聊

#define QUIT -1   // 退出
#define OK 100
#define ERROR -100

struct sockaddr_in serviceSin;

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
    char data[1024];  //可作为用户聊天内容，也可作为传入验证信息
    char user[1024];  //用于传输用户信息
    char friendData[1024];
    char temp[1024];
} state_t;

int serviceInit();

//主结构IO复用
void struMain(int fd);



#endif