#include "tank_link.h"

link_handler_t my_link_handler;

int tank_link_msg_send(void)
{
    int id = msgget(1232, IPC_CREAT|0666);
    if (id == -1){
        printf("error, key has exist\n");
    }else{
        printf("key has no use, msg_id:%d\n",id);
    }
    struct msqid_ds buf;
    my_msg_t test = {1,"huangwankuan"};
    int count = msgctl(id,IPC_STAT,&buf);
    uint32_t num_messages = buf.msg_qnum;

    printf("Number of messages = %d\n",num_messages);
    // memset(test.string,'\0',sizeof(((my_msg_t*)0)->string));
    // test.type = 0;
    // strncpy(test.string,"huangwankuan",200);
    msgsnd(id, &test, sizeof(((my_msg_t*)0)->string), 0);
    printf("send message is %s\n",test.string);
    // my_msg_t msg[5] = {
    //     {1, "Luffy"},
    //     {1, "Zoro"},
    //     {2, "Nami"},
    //     {2, "Usopo"},
    //     {3, "huang"},
    // };
    // int i;
    // for (i = 0; i < 5; ++i) {
    //     // 这里为什么-20？
    //     // 发送的缓冲区大小和接收一致时，会出现stack smashing detected 溢出错误
    //     int res = msgsnd(id, &msg[i], sizeof(((my_msg_t*)0)->string)-20, 0);
    //     printf("send message is %s\n",msg[i].string);
    //     printf("send res is %d\n", res);
    // }
    return 0;
}


int tank_link_msg_rev(void)
{
    // 获取 ipc 内核对象 id
    int id = msgget(1232, 0);
    ssize_t res = 0;
    my_msg_t msg;
    while(1) {
        // 以非阻塞的方式接收类型为 type 的消息
        res = msgrcv(id, &msg, sizeof(((my_msg_t*)0)->string), 0, MSG_NOERROR|IPC_NOWAIT);
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



