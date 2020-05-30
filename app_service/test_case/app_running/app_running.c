#include "tank_app_pub.h"
#include "tank_app.h"
#include "pthread.h"
#include "tcp_fsm.h"
#include "tank_ID.h"
#include "tank_map.h"
#include <pthread.h>
#include "tank_log.h"
#include "tank_delay.h"
#include "tank_app_api.h"


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


tank_status_t tank_app_recv_package_callback(app_package_info_t* info)
{
    static uint32_t num;
    memcpy(&num, info->package, info->size);
    log_info("=======get a messgae from src_id:%d, size:%d, info:%d=======\n",
            info->src_id, info->size, num);
    if(info->src_id == 4){
        log_info("============send a msg num:%d============\n", num);
        ta_send_package(&app_demo2, info->src_id, &num, sizeof(num), 0);
    }

    return TANK_SUCCESS;
}

int count = 0;
int main(int argc, char *argv[])
{
    // pthread_t pid;
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
            tank_app_creat(&app_demo, src_id, 0, 0, RECV_SYNC);
            // for(int i=0;i<10;++i){
            //     if(i != app_demo.id){
            //         tank_app_listen(&app_demo, i);
            //     }
            // }

            while(1){
                static uint32_t num = 0;
                log_info("============send a msg num:%d============\n", num);
                ta_send_package(&app_demo, dst_id, &num, sizeof(num), 0);
                sleep_ms(1000);
                num += 1;
            }
            tank_app_destory(&app_demo);
        }else if(!strncmp(argv[1], "recv", 1024)){
            tank_log_init(&mylog, "app_reciver",2048, LEVEL_INFO,
                    LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                    PORT_FILE|PORT_SHELL
                    );
            log_info("========logger start===========\n");
            tank_app_creat(&app_demo, 2, 0, 0, RECV_ASYNC);
            tank_app_creat(&app_demo2, 3, 0, 0, RECV_SYNC);
            for(int i=0;i<10;++i){
                if(i != app_demo.id){
                    tank_app_listen(&app_demo, i);
                }
            }
            for(int i=0;i<10;++i){
                if(i != app_demo2.id){
                    tank_app_listen(&app_demo2, i);
                }
            }
            // pthread_create(&pid, NULL, recv, &app_demo);
            // pthread_create(&pid, NULL, recv, &app_demo2);

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
