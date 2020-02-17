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

#define MSGQ_PACKET_MAX_SIZE 100
#define MSGQ_BUF_SIZE 50


#define msgq_receiver_ALL 0

my_msgq_t app_sender;
my_msgq_t app_receiver;

uint8_t send_buffer[10][MSGQ_PACKET_MAX_SIZE]={
    "1st message",
    "2nd message",
    "3rd message",
    "4th message",
    "5th message",
    "6th message",
    "7th message",
    "8th message",
    "9th message",
    "10th message",
    };
int main(int agr, char *argv[])
{
    pthread_t pthread_rcv;

    msgq_constuctor_id(&app_sender,0x100);
    msgq_constuctor_id(&app_receiver,0x101);
    for(int i=0;i<10;i++){
        msgq_send(&app_sender, send_buffer[i],MSGQ_PACKET_MAX_SIZE,i+1);
    }
    msgq_destuctor(&app_sender);
    msgq_destuctor(&app_sender);
    return 0;
}