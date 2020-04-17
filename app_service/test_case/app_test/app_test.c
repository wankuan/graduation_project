#include "tank_app_pub.h"
#include "tank_app.h"
#include "pthread.h"
#include "tcp_fsm.h"
#include "tank_ID.h"
#include "tank_map.h"
#include <pthread.h>
#include "tank_log.h"
#include "tank_delay.h"

ta_info_t app_demo;
ta_info_t app_demo2;

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


void *recv(void *arg)
{
    ta_info_t *ta = (ta_info_t *)arg;
    while(1){
        tank_id_t src_id;
        uint16_t size;
        uint32_t num = 0;
        ta_recv_package(ta, &src_id, &num, &size, 100);
        log_info("=======get a messgae from src_id:%d, size:%d, info:%d=======\n", src_id, size, num);
        // uint16_t cur_index = strlen(buf);
        // buf[cur_index] = num + '0';
        // buf[cur_index+1] = '\0';
        // ta_send_package(ta, src_id, buf, cur_index+2, 0);
        // sleep(1);
    }
}

int count = 0;
int main(int argc, char *argv[])
{
    pthread_t pid;
    uint32_t value = 2048;
    printf("value %x, mask %x slice %x\n", value, PACKAGE_MASK, PACKAGE_SLICE(value));
    if(argc == 4){
        long src_id = atol(argv[2]);
        long dst_id = atol(argv[3]);
        if(!strncmp(argv[1], "send", 1024)){
            tank_log_init(&mylog, "app_sender",2048, LEVEL_INFO,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            log_info("sender\n");
            tank_app_creat(&app_demo, src_id, 0, 0);

            sleep(1);
            write_tcp_state(&app_demo, dst_id, SYN_SENT);
            tank_app_tcp_send(&app_demo, dst_id, TCP_SYN);

            // char buf_test[100] = "hello, my guy!";
            // ta_send_package(&app_demo, dst_id, buf_test, 30, 0);

            while(1){
                static uint32_t num = 0;
                ta_send_package(&app_demo, dst_id, &num, sizeof(num), 0);
                log_info("============send a msg num:%d============\n", num);
                sleep_ms(100);
                num += 1;
            }
            tank_app_destory(&app_demo);
        }else if(!strncmp(argv[1], "recv", 1024)){
            tank_log_init(&mylog, "app_reciver",2048, LEVEL_INFO,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            tank_app_creat(&app_demo, src_id, 0, 0);
            tank_app_creat(&app_demo2, 2, 0, 0);

            pthread_create(&pid, NULL, recv, &app_demo);
            pthread_create(&pid, NULL, recv, &app_demo2);

            tank_app_destory(&app_demo);
            goto exit;
        }else{
            goto error;
        }
    }else{
        goto error;
    }
exit:

    log_info("end running\n");
    return 0;
error:
    log_error("input error\n ./a.out send/recv id_self id_dst\n");
    return -1;
}
