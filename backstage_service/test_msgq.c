#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "my_msgq.h"

int main(int argc, char *argv[])
{
    printf("Compile time:%s %s\n", COMPILE_DATE, COMPILE_TIME);
    printf("GCC version:%s\n", GCC_VERSION);
    printf("--------Git--------\n%s\n",GIT_ALL);
    my_msgq_t msg;
    msgq_constuctor(&msg);
    if(argc == 2){
        if(*argv[1]=='s'){
            char message[100]="[sender]:This is the sender";
            printf("send message is\n");
            printf("%s\n",message);
            msgq_send(&msg, message, 100, 1);
        }else if(*argv[1]=='r'){
            char message[100]="\0";
            msgq_rcv_wait(&msg, message, 100, 0);
            printf("receiver message is\n");
            printf("%s\n",message);
        }else{
            printf("error input!\n");
        }
    }else{
        printf("----HOW TO RUN----\n");
        printf("s-send\nt-receive\n");
        }

    return 0;
}
