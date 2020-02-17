#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "config.h"
#include "my_sem.h"
#define TEST_SIZE 257

int sem_id;


char message[100]="\0";

void* print1(void *arg)
{
    printf("fun1 pid is %ld\n",pthread_self());
    printf("fun1 getpid is %d\n",getpid());
    printf("wait for get the message\n");
    printf("[fun1]current: num:%d\n",get_nsem(sem_id,0));
    sem_p(sem_id);
    printf("[fun1]message is %s\n",message);
    printf("[fun1]current: num:%d\n",get_nsem(sem_id,0));
    sem_v(sem_id);
    printf("[fun1]current: num:%d\n",get_nsem(sem_id,0));
}


void* print2(void *arg)
{
    printf("fun2 pid is %ld\n",pthread_self());
    printf("fun2 getpid is %d\n",getpid());
    printf("after 5 seconds, will wirte into message\n");
    sleep(5);
    strncpy(message,"huang wan kuan",100);
    printf("has written into message, check\n");
    sem_v(sem_id);
    printf("[fun2]current: num:%d\n",get_nsem(sem_id,0));
}
int main(int argc, char *argv[])
{
    pthread_t p_pid;
    printf("Compile time:%s %s\n", COMPILE_DATE, COMPILE_TIME);
    printf("GCC version:%s\n", GCC_VERSION);
    printf("--------Git--------\n%s\n",GIT_ALL);
    printf("process pid is %d\n",getpid());
    sem_id = my_sem_init(1);
    printf("current: num:%d\n",get_nsem(sem_id,0));
    sem_p(sem_id);
    printf("current: num:%d\n",get_nsem(sem_id,0));
    pthread_create(&p_pid,NULL,&print1,NULL);
    pthread_create(&p_pid,NULL,&print2,NULL);

    pthread_join(p_pid,NULL);
    printf("status:%d\n", rm_sem(sem_id, 0));
    return 0;
}

