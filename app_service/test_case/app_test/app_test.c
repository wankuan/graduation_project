#include "tank_app_pub.h"
#include "tank_app.h"
#include "pthread.h"
#include "tcp_fsm.h"

ta_info_t app_demo;

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


void *fsm_thread(void *arg)
{
    while(1){
        tank_fsm_recv(&app_demo);
    }
}
int count = 0;
int main(int argc, char *argv[])
{
    pthread_t my_pid_t;
    if(argc == 2){
        if(!strncmp(argv[1], "send", 1024)){
            tank_log_init(&mylog, "app_sender",2048, LEVEL_INFO,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            log_info("sender\n");
            tank_app_creat(&app_demo, 1, 0, 0);
                // for(int i=0;i<4;i++){
                //     tank_app_send(&app_demo, 1, i);
                // }
                // sleep(1);
            app_demo.index_lut[0].id = 2;
            app_demo.index_lut[0].index = 0;
            write_tcp_state(&app_demo, 2, SYN_SENT);
            pthread_create(&my_pid_t,NULL,&fsm_thread,NULL);
            tank_app_send(&app_demo, 2, TCP_SYN);
            pthread_join(my_pid_t,NULL);
        }else if(!strncmp(argv[1], "recv", 1024)){
            tank_log_init(&mylog, "app_reciver",2048, LEVEL_INFO,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            tank_app_creat(&app_demo, 2, 0, 0);
            // tank_id_t src_id = 0;
            // tcp_state_t state = 0;
            // tank_app_recv_wait(&app_demo, &src_id, &state);
            app_demo.index_lut[0].id = 1;
            app_demo.index_lut[0].index = 0;
            write_tcp_state(&app_demo, 1, LISTEN);
            pthread_create(&my_pid_t,NULL,&fsm_thread,NULL);
            pthread_join(my_pid_t,NULL);
            goto exit;
        }else{
            goto error;
        }
    }else{
        goto error;
    }
exit:
    // pthread_create(&my_pid_t,NULL,&test2,NULL);
    // // pthread_create(&my_pid_t,NULL,&test3,NULL);
    // pthread_join(my_pid_t,NULL);
    log_info("end running\n");
    return 0;
error:
    log_error("input error\n ./a.out send/recv\n");
    return -1;
}
