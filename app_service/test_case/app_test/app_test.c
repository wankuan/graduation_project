#include "tank_app_pub.h"
#include "tank_app.h"
#include "pthread.h"
#include "tcp_fsm.h"
#include "tank_ID.h"

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
    uint32_t value = 2048;
    printf("value %x, mask %x slice %x\n", value, PACKAGE_MASK, PACKAGE_SLICE(value));
    if(argc == 4){
        long src_id = atol(argv[2]);
        long dst_id = atol(argv[3]);
        if(!strncmp(argv[1], "send", 1024)){
            tank_log_init(&mylog, "app_sender",2048, LEVEL_DEBUG,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            log_info("sender\n");
            tank_app_creat(&app_demo, src_id, 0, 0);
                // for(int i=0;i<4;i++){
                //     tank_app_send_msg(&app_demo, 1, i);
                // }
                // sleep(1);
            // app_demo.index_lut[0].id = dst_id;
            // app_demo.index_lut[0].index = 0;
            // app_demo.id_cur_index += 1;
            // write_tcp_state(&app_demo, dst_id, SYN_SENT);
            // log_info("start TCP shake hands\n");
            pthread_create(&my_pid_t,NULL, (void*)&send_package_thread,&app_demo);
            pthread_create(&my_pid_t,NULL, (void*)&recv_thread,&app_demo);
            sleep(1);
            // tank_app_send_msg(&app_demo, dst_id, TCP_SYN);
            // sleep(2);
            char buf_test[] = "hello, my guy!";
            ta_send_package(&app_demo, dst_id, buf_test, 30, 0);
            // log_info("start TCP closed\n");
            // write_tcp_state(&app_demo, dst_id, FIN_WAIT_1);
            // tank_app_send_msg(&app_demo, dst_id, TCP_FIN);

            pthread_join(my_pid_t,NULL);
        }else if(!strncmp(argv[1], "recv", 1024)){
            tank_log_init(&mylog, "app_reciver",2048, LEVEL_DEBUG,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            tank_app_creat(&app_demo, src_id, 0, 0);
            for(int i=0;i<10;++i){
                if(i != src_id){
                    tank_app_listen(&app_demo, i);
                }
            }
            // tank_id_t src_id = 0;
            // tcp_state_t state = 0;
            // tank_app_recv_msg_wait(&app_demo, &src_id, &state);
            // app_demo.index_lut[0].id = 1;
            // app_demo.index_lut[0].index = 0;
            // write_tcp_state(&app_demo, dst_id, LISTEN);
            // log_info("start TCP shake hands\n");

            pthread_create(&my_pid_t,NULL, (void*)&recv_thread,&app_demo);
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
    log_error("input error\n ./a.out send/recv id_self id_dst\n");
    return -1;
}
