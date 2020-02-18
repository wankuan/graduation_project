#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "config.h"
#include "my_shm.h"
#include "my_sem.h"
#include "my_msgq.h"
#include <sys/time.h>
#include "my_log.h"
#define MSGQ_PACKET_MAX_SIZE 100
#define MSGQ_BUF_SIZE 50


#define msgq_receiver_ALL 0

my_msgq_t msgq_receiver;
my_msgq_t msgq_sender;

uint8_t rcv_buf[MSGQ_BUF_SIZE][MSGQ_PACKET_MAX_SIZE];


void get_current_time(uint8_t *p_timer);

void* rcv_thread(void *arg)
{
    uint8_t time_buf[100];
    uint8_t buf[MSGQ_PACKET_MAX_SIZE];
    long type = 0;
    while(msgq_rcvall_wait(&msgq_receiver, (void*)buf, MSGQ_PACKET_MAX_SIZE,&type)==MSGQ_SUCCESS)
    {
        get_current_time(time_buf);
        memcpy(rcv_buf[type],buf,MSGQ_PACKET_MAX_SIZE);
        LOG_S("[UNIX_SERVICE]:%s\n",rcv_buf[type]);
    }
}

int main(int agr, char *argv[])
{
    // char *buf=make_message("huang wankuan  %d",777);
    // printf("%s\n",buf);
    char *buf = my_printf("523");
    printf("%c %c %c %c ",buf[0],buf[1],buf[2],buf[3]);
    // pthread_t pthread_rcv;
    // msgq_constuctor_id(&msgq_receiver,0x100);
    // msgq_constuctor_id(&msgq_sender,0x101);

    // pthread_create(&pthread_rcv,NULL,&rcv_thread,NULL);

    // pthread_join(pthread_rcv,NULL);

    // msgq_destuctor(&msgq_receiver);
    // msgq_destuctor(&msgq_sender);
    return 0;
}