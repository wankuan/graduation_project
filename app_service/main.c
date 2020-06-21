#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "tank_app.h"
#include "tank_link.h"
#define TEST_SIZE 257

int main(int argc, char *argv[])
{
    printf("Proejct Version:%s\n",PROJECT_VERSION);
    printf("Compile time:%s %s\n", COMPILE_DATE, COMPILE_TIME);
    printf("GCC version:%s\n", GCC_VERSION);
    printf("--------Git--------\n%s\n",GIT_ALL);
    // tank_app_t APP;
    // tank_app_init(&APP);
    // const char *msg_send = "huang";
    // char *msg_send_test_buf[TEST_SIZE];
    // for(uint16_t i = 0; i<TEST_SIZE; i++){
    //     msg_send_test_buf[i]=(char*)malloc(256);
    //     snprintf(msg_send_test_buf[i],256,"%s_%d",msg_send,i);
    //     tank_app_send_msg(&APP, (const uint8_t*)(msg_send_test_buf[i]), (strlen(msg_send_test_buf[i])+sizeof("")));
    // }
    // for(uint16_t i = 0; i<TEST_SIZE; i++){
    //     free(msg_send_test_buf[i]);
    // }
    if(argc == 2){
        if(*argv[1]=='1'){
            printf("send messgae!\n");
            tank_link_msg_send();
        }else if(*argv[1]=='2'){
            printf("receive message!\n");
            tank_link_msg_rev();
        }else{
            printf("1-send\n2-receive\n");
        }
    }
    return 0;
}

