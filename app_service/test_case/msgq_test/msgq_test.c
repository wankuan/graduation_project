#include <pthread.h>

#include "tank_pub.h"
#include "tank_msgq.h"

my_sem_t sem1;


char message[100];
void* print1(void *arg)
{
    printf("fun1 pid is %ld\n",pthread_self());
    printf("fun1 getpid is %d\n",getpid());
    printf("wait for get the message\n");
    my_sem_wait(&sem1);
    printf("[fun1]message is %s\n",message);
    my_sem_post(&sem1);
}


void* print2(void *arg)
{
    printf("fun2 pid is %ld\n",pthread_self());
    printf("fun2 getpid is %d\n",getpid());
    printf("after 5 seconds, will wirte into message\n");
    sleep(5);
    strncpy(message,"huang wan kuan",100);
    printf("has written into message, check\n");
    my_sem_post(&sem1);
}
// int main(int argc, char *argv[])
// {
//     pthread_t p_pid;
//     printf("process pid is %d\n",getpid());
//     my_sem_creat(&sem1, 1);
//     uint8_t val;
//     my_sem_get_val(&sem1, &val);
//     printf("current: num:%d\n", val);
//     my_sem_wait(&sem1);
//     my_sem_get_val(&sem1, &val);
//     printf("current: num:%d\n", val);
//     pthread_create(&p_pid,NULL,&print1,NULL);
//     pthread_create(&p_pid,NULL,&print2,NULL);

//     pthread_join(p_pid,NULL);
//     my_sem_destroy(&sem1);
//     return 0;
// }

int main(int argc, char *argv[])
{
    tank_msgq_t msgq;
    tank_msgq_creat(&msgq, 10);
    strncpy(msgq.buf[0], "huang", 10);
    strncpy(msgq.buf[1], "huang1", 10);
    strncpy(msgq.buf[2], "huang2", 10);
    printf("%p\n",&msgq.buf[0]);
    printf("%p\n",&msgq.buf[1]);
    printf("%p\n",msgq.buf[2]);
    printf("%s\n",msgq.buf[0]);
    printf("%s\n",msgq.buf[1]);
    printf("%s\n",msgq.buf[2]);
    tank_msgq_delete(&msgq);
    return 0;
}