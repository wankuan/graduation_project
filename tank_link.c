#include "tank_link.h"

int tank_link_msg_send(void)
{
    int id = msgget(0x8888, IPC_CREAT|0666);
    my_msg_t msg[5] = {
        {1, "Luffy"},
        {1, "Zoro"},
        {2, "Nami"},
        {2, "Usopo"},
        {3, "huang"},
    };
    int i;
    for (i = 0; i < 5; ++i) {
        int res = msgsnd(id, &msg[i], sizeof(((my_msg_t*)0)->string), 0);
        printf("send message is %s\n",msg[i].string);
        printf("send res is %d\n", res);
    }
    return 0;
}


int tank_link_msg_rev(void)
{
    // 获取 ipc 内核对象 id
    int id = msgget(0x8888, 0);
    int res = 0;
    my_msg_t msg;
    while(1) {
        // 以非阻塞的方式接收类型为 type 的消息
        res = msgrcv(id, &msg, sizeof(((my_msg_t*)0)->string), 0, IPC_NOWAIT);
        if (res < 0) {
            // 如果消息接收完毕就退出，否则报错并退出
            if (errno == ENOMSG) {
                printf("No message!\n");
                break;
            }
        }
        // 打印消息内容
        printf("message is %s\n",msg.string);
    }
    return 0;
}



