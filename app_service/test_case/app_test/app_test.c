#include "tank_app_pub.h"
#include "tank_app.h"
#include "pthread.h"
ta_info_t app[10];

#include "tank_log_api.h"
#define FILE_NAME "app_test"

char name_buf[10][8]={
    "app-0",
    "app-1",
    "app-2",
    "app-3",
    "app-4",
    "app-5",
    "app-6",
    "app-7",
    "app-8",
    "app-9",
};





int len = sizeof(name_buf)/sizeof(name_buf[0]);

void *test1(void *arg)
{
    for(int i=0;i<2;++i){
    tank_app_creat(&app[i], i, 0, 0);
    log_info("\n\n\n");
    }
    return NULL;
}


void *test2(void *arg)
{
    sleep(3);
    while(1){
        for(int i=0;i<25;i++){
            tank_app_send(&app[0], 1, SYN_SENT);
        }
        sleep(10);
    }
}
int count = 0;
int main(int argc, char *argv[])
{
    pthread_t my_pid_t;
    if(argc == 2){
        if(!strncmp(argv[1], "send", 1024)){
            tank_log_init(&mylog, "app_sender",2048, LEVEL_DEBUG,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            log_info("sender\n");
            tank_app_creat(&app[0], 0, 0, 0);
            while(1){
                for(int i=0;i<10;i++){
                    tank_app_send(&app[0], 1, i);
                }
                sleep(1);
            }
        }else if(!strncmp(argv[1], "recv", 1024)){
            tank_log_init(&mylog, "app_reciver",2048, LEVEL_DEBUG,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            log_info("receiver\n");
            tank_app_creat(&app[0], 1, 0, 0);
            while(1){
                tank_id_t src_id = 0;
                tcp_state_t state = 0;
                tank_app_recv_wait(&app[0], &src_id, &state);
            }
            goto exit;
        }else{
            goto error;
        }
    }else{
        goto error;
    }
exit:

    // pthread_create(&my_pid_t,NULL,&test1,NULL);
    // pthread_create(&my_pid_t,NULL,&test2,NULL);
    // // pthread_create(&my_pid_t,NULL,&test3,NULL);

    // pthread_join(my_pid_t,NULL);
    log_info("end running\n");
    return 0;
error:
    log_error("input error\n ./a.out send/recv\n");
    return -1;
}
