#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <pthread.h>

#include "config.h"
#include "my_sem.h"
#include "osal_pub.h"
#define TEST_SIZE 257

my_sem_t sem;

char message[100]="\0";

void* thread1(void *arg)
{
    printf("[thread1]wait for get the message\n");
    sem_p(&sem);
    printf("[thread1]receive:\n");
    printf("[thread1]%s\n",message);
    sem_v(&sem);
}


void* thread2(void *arg)
{
    printf("[thread2]sleep 3s and send message\n");
    sleep(3);
    strncpy(message,"semaphore is running!",100);
    printf("[thread2]write message:\n");
    printf("[thread2]%s\n",message);
    sem_v(&sem);
}
int main(int argc, char *argv[])
{
    pthread_t pthread1, pthread2;

    sem_constuctor(&sem);
    sem_set_val(&sem,1);
    sem_p(&sem);

    pthread_create(&pthread1,NULL,&thread1,NULL);
    pthread_create(&pthread2,NULL,&thread2,NULL);
    pthread_join(pthread1,NULL);
    pthread_join(pthread2,NULL);

    sem_destuctor(&sem);
    return 0;
}