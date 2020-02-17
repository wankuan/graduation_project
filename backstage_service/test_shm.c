#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "my_shm.h"


int main(int argc, char *argv[])
{
    my_shm_t shm;
    shm_constuctor(&shm);
    if(argc == 2){
        if(*argv[1]=='s'){
            char message[100]="[WRITE]:This is the sender";
            shm_write(&shm,message,100);
            printf("write message is\n");
            printf("%s\n",message);
        }else if(*argv[1]=='r'){
            char message[100]="\0";
            shm_read(&shm,message,100);
            printf("read message is\n");
            printf("%s\n",message);
            shm_delete(&shm);
        }else{
            printf("error input!\n");
        }
    }else{
        printf("----HOW TO RUN----\n");
        printf("s-send\nt-receive\n");
        }
    shm_destuctor(&shm);
    return 0;
}
