#include "workClient.h"

int main(int argc, char const *argv[])
{
    // 客户端初始化
    int fd = clientInit();
    // 首先创建子进程，不知道是否要用信号量控制
    // 信号量初始化 
    int semid = 0;
    chatCon_t * head = creatHead();
    mastControl(fd , head);



    return 0;
}
