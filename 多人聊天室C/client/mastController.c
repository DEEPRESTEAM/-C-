#include "workClient.h"

// 主界面功能集成
void mastControl(int fd, chatCon_t *head)
{
    while (1)
    {
        sleep(2);
        system("clear");
        uiInit("./UI/MuUI.txt");
        int funId = 0;
        printf("选择你的功能:");
        scanf("%d", &funId);
        getchar();
        user_t *user = malloc(sizeof(user_t));
        switch (funId)
        {
        case 1:
            if ((user = Login(fd)) != NULL)
            {
                sleep(1);
                secondController(fd, user, head);
            }
            else
            {
                break;
            }
            break;
        case 2:
            registe(fd);
            break;
        case 3:
            forgetPwd(fd);
            break;
        case 4:
            // 正常退出程序
            exit(0);
        default:
            printf("输入错误，请重新输入\n");
            break;
        }
    }
}

// 二级控制台
void secondController(int fd, user_t *user, chatCon_t *head)
{
    while (1)
    {
        uiInit("./UI/SeUI.txt");
        pid_t pid = 0;
        int funId = 0;
        printf("选择你的功能:");
        scanf("%d", &funId);
        printf("fubid =%d\n" , funId);
        switch (funId)
        {
        case 1:
            allChat(fd, user, head);
            system("clear");
            break;
        case 2:
            friendModle(fd , user , head);
            break;
        case 3:
            setFriendChat(fd , user);
            break;
        case 4:
        joinFriendChat(fd , user);
            break;
        case 5:
            printf("————————————————查看历史记录——————————————\n");
            prinChatCon(head);
            break;
        case 6:
            // 退出上一级 ， 即退出登录
            if (quitLogin(fd, user) == 1)
            {
                return;
            }
            break;
        default:
            printf("输入错误\n");
            break;
        }
    }
}

// 三级控制台
void friendModle(int fd , user_t * user , chatCon_t * head)
{
    while (1)
    {
        uiInit("./UI/FriendUI.txt");
        pid_t pid = 0;
        int funId = 0;
        printf("选择你的功能:");
        scanf("%d", &funId);
        getchar();
        switch (funId)
        {
        case 1:
            showFriend(fd , user);
            break;
        case 2:
            addFriend(fd, user);
            break;
        case 3:
            deleteFriend(fd , user);
            break;
        case 4:
            addFriendRequest(fd , user);
            break;
        case 5:
            friendChat(fd , user , head);
            break;
        case 6:
            return;
            break;
        default:
            break;
        }
    }
    
    
}