#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"
#include "inner_service.h"
#include "tank_delay.h"


#include "tank_log_api.h"
#define FILE_NAME "inner_service"

#define HEART_BEAT_TIME         1
#define HEART_BEAT_LOST_TIME    2

//TODD:短消息带ACK才行
// APP所有信息的对应表
app_info_t               app_info_table[256];
// 当前系统占用的ID数
uint16_t                 g_app_id_cur_size = 0;
// 在本系统的ID
tank_id_t                app_id_lut[10] = {0,1,2,3,4,5,6,7,8,9};
// 内部堆管理结构体
tank_mm_t                g_inner_service_mm;
// 线程PID
pthread_t                g_service_pid;
// 后台接收消息队列
tank_msgq_t             *g_service_request_msgq;
// 推送后台给APP分配的内存
inner_service_push_heap_t          *g_push_heap_t;

app_package_t app_package_info[16*1024] = {0};

// 全局packgae ID，每发起packgae将会增加1
static uint32_t       g_app_package_seq = 0;


static tank_status_t app_malloc_init(void);
static app_package_t* find_package_info(tank_id_t package_id);
static void print_app_info_table(void);
static int find_id_index(tank_id_t id);
void *handler_thread(void *arg);



static tank_status_t get_cur_time(time_t *t)
{
  time(t);
  return TANK_SUCCESS;
}



static app_package_t* find_package_info(tank_id_t package_id)
{
    for(int i=0;i<g_app_package_seq;++i){
        if(app_package_info[i].package_id == package_id){
            log_debug("find package_id id:%d\n", package_id);
            return &app_package_info[i];
        }
    }
    return NULL;
}

static void print_app_info_table(void)
{
    log_info("=========app_info_table============\n");
    for(int i=0;i<g_app_id_cur_size;++i){
        log_info("id:%d, heap:%p, msgq_r:%p, msgq_s:%p\n",
        app_info_table[i].id, app_info_table[i].heap_addr,
        app_info_table[i].msgq_recv_addr, app_info_table[i].msgq_send_addr
        );
    }
    log_info("=========app_info_table============\n");
}


tank_status_t inner_service_init(void)
{
    tank_creat_shm();
    log_info("start addr:0x%x\n", g_shm_base);
    log_info("inner swap addr:0x%x\n", INNER_SWAP_ADDR);
    tank_mm_register(&g_inner_service_mm, INNER_SWAP_ADDR, INNER_SWAP_SIZE, "inner_service_mm");
    app_malloc_init();
    pthread_create(&g_service_pid,NULL,&handler_thread,NULL);
    return TANK_SUCCESS;
}

tank_status_t inner_service_deinit(void)
{
    pthread_join(g_service_pid,NULL);
    return TANK_SUCCESS;
}
/*kill */
static tank_status_t app_malloc_init(void)
{
    g_push_heap_t = (inner_service_push_heap_t *)SEM_ADDR;
    log_info("sem addr:%p  size:%d\n", &g_push_heap_t->sem, sizeof(my_sem_t));

    sem_destroy(&g_push_heap_t->sem);
    my_sem_creat(&g_push_heap_t->sem, 0);

    g_service_request_msgq = (tank_msgq_t*)tank_mm_malloc(&g_inner_service_mm, sizeof(tank_msgq_t)+50*20);
    if(g_service_request_msgq == NULL){
        log_error("g_service_request_msgq allocate memory fail, heap full\n");
    }
    log_info("msgq addr:%p\n", g_service_request_msgq);

    *(uint32_t*)MSGQ_MAP_ADDR = (uint32_t)g_service_request_msgq - g_shm_base;
    tank_msgq_creat(g_service_request_msgq, 20, 50);
    log_info("========inner service init OK===========\n");
    return TANK_SUCCESS;
}

static int find_id_index(tank_id_t id)
{
    for(int i=0;i<g_app_id_cur_size;++i){
        if(app_info_table[i].id == id){
            log_debug("find id index:%d\n", i);
            return i;
        }
    }
    return -1;
}


tank_status_t check_systemid(tank_id_t id)
{
    tank_id_t len = sizeof(app_id_lut)/sizeof(app_id_lut[0]);
    for(int i=0;i<len;++i){
        if(app_id_lut[i] == id){
            return TANK_SUCCESS;
        }
    }
    log_error("id:%d not in current system\n", id);
    return TANK_FAIL;
}


void *hear_beat_send_thread(void *arg)
{
    app_request_info_t info;
    while(1){
        for(int i=0;i<g_app_id_cur_size;++i){
            memset(&info, 0, TANK_MSG_NORMAL_SIZE);
            info.type = HEART_BEAT;
            info.heart_beat.src_id = 0;
            info.heart_beat.value = 0;
            time_t cur_time;
            get_cur_time(&cur_time);
            if((cur_time - app_info_table[i].last_refresh) >= HEART_BEAT_LOST_TIME){
                log_error("app_id:%d has no response\n", app_info_table[i].id);
            }else{
                tank_msgq_send((tank_msgq_t*)app_info_table[i].msgq_recv_addr, &info, TANK_MSG_NORMAL_SIZE);
            }
            sleep_ms(5);
        }
        sleep_ms(500);
    }
}


void *handler_thread(void *arg)
{
    app_request_info_t info;
    while(1){
        memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
        tank_msgq_recv_wait(g_service_request_msgq, &info, TANK_MSGQ_NORMAL_SIZE);
        if(info.type == MM_ALLOCATE){
            log_info("======MM_ALLOCATE======\n");
            log_info("src_id:%d, request size:%d\n", info.heap.id, info.heap.size);
            int index = find_id_index(info.heap.id);
            if(index >= 0){
                log_error("src_id:%d has been malloc\n", info.heap.id);
                continue;
            }
            void *addr = tank_mm_calloc(&g_inner_service_mm, info.heap.size);
            if(addr == NULL){
                log_error("addr allocate memory fail, heap full\n");
            }
            log_info("inner_service heap remain:%d\n", g_inner_service_mm.heap.xFreeBytesRemaining);
            uint32_t shift = (uint32_t)addr - g_shm_base;

            app_info_table[g_app_id_cur_size].id = info.heap.id;
            app_info_table[g_app_id_cur_size].heap_addr = (void*)addr;
            get_cur_time(&app_info_table[g_app_id_cur_size].last_refresh);
            log_info("src_id:%d, shift:%d\n", info.heap.id, shift);
            g_push_heap_t->shift = shift;
            g_app_id_cur_size += 1;
            log_info("======MM_ALLOCATE EXIT======\n");
            my_sem_post(&g_push_heap_t->sem);
        }else if(info.type == APP_PUSH_MSGQ_ADDR){
            log_info("======APP_PUSH_MSGQ_ADDR======\n");
            log_info("src_id:%d, receiver_shift:%d, send_shift:%d\n", info.msgq.id, info.msgq.recv_shift, info.msgq.send_shift);
            int index = find_id_index(info.msgq.id);
            if(index < 0){
                log_error("can not find id:%d\n", info.msgq.id);
                continue;
            }
            app_info_table[index].msgq_recv_addr = (void*)info.msgq.recv_shift + g_shm_base;
            app_info_table[index].msgq_send_addr = (void*)info.msgq.send_shift + g_shm_base;
            log_info("inner_service get app msgq info successfully\n");
            log_info("======APP_PUSH_MSGQ_ADDR EXIT======\n");
            print_app_info_table();
        }else if(info.type == APP_TCP_MSG){
            log_info("======APP_TCP_MSG======\n");
            int index = find_id_index(info.tcp_state.dst_id);
            if(index < 0){
                log_error("can not find id:%d\n", info.tcp_state.dst_id);
                continue;
            }
            tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &info, APP_MSG_SIZE);
            log_info("restransmit src_id:%d, dst_id:%d, flag:%d\n",
                    info.tcp_state.src_id, info.tcp_state.dst_id, info.tcp_state.flag
                    );
            log_info("======APP_TCP_MSG EXIT======\n");
        }else if(info.type == APP_SEND_PACKAGE_REQUEST){
            log_info("=====package transmit start======\n");
            log_info("package: 1st request\n");
            int index = find_id_index(info.send_package_request.dst_id);
            if(index < 0){
                log_error("can not find id:%d\n", info.send_package_request.dst_id);
                continue;
            }
            static uint32_t g_package_id = 0;
            app_package_info[g_app_package_seq].src_id = info.send_package_request.src_id;
            app_package_info[g_app_package_seq].dst_id = info.send_package_request.dst_id;
            app_package_info[g_app_package_seq].package_id = g_package_id;
            app_package_info[g_app_package_seq].size = info.send_package_request.size;
            app_package_info[g_app_package_seq].state = REQUEST_MM;
            void *addr = tank_mm_calloc(&g_inner_service_mm, info.send_package_request.size);
            if(addr == NULL){
                log_error("addr allocate memory fail, heap full\n");
                continue;
            }
            uint32_t add_shift = (uint32_t)addr - g_shm_base;
            app_package_info[g_app_package_seq].addr_shift = add_shift;

            log_info("ALLOCATE INFO:src_id:%d, dst_id:%d, package_id:%d, size:%d, addr_shift:%d\n",
                    app_package_info[g_app_package_seq].src_id, app_package_info[g_app_package_seq].dst_id,
                    app_package_info[g_app_package_seq].package_id, app_package_info[g_app_package_seq].size,
                    app_package_info[g_app_package_seq].addr_shift
                    );

            memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
            log_info("package: 2nd allocate\n");
            index =  find_id_index(app_package_info[g_app_package_seq].src_id);
            info.type = APP_GET_PACKAGE_ALLOCATE;
            info.recv_package_allocate.addr_shift = add_shift;
            info.recv_package_allocate.dst_id = app_package_info[g_app_package_seq].src_id;
            info.recv_package_allocate.package_id = app_package_info[g_app_package_seq].package_id;
            tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &info, TANK_MSG_NORMAL_SIZE);

            g_app_package_seq += 1;
            g_package_id += 1;
        }else if(info.type == APP_SEND_PACKAGE_FINISHED){
            log_info("package: 3rd get sender finished ACK\n");
            uint32_t package_id = info.send_package_finished.package_id;
            app_package_t  *package_info = find_package_info(package_id);
            package_info->state = RESTRANSIMTING;
            if(package_info == NULL){
                log_error("can not find package_id:%d\n", package_id);
                continue;
            }

            memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
            info.type = APP_recv_package_push;
            info.recv_package_push.src_id = package_info->src_id;
            info.recv_package_push.dst_id = package_info->dst_id;
            info.recv_package_push.package_id = package_info->package_id;
            info.recv_package_push.addr_shift = package_info->addr_shift;
            info.recv_package_push.size = package_info->size;
            tank_id_t index =  find_id_index(info.recv_package_push.dst_id);
            if(tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &info, TANK_MSG_NORMAL_SIZE) == TANK_FAIL){
                log_error("package: 4th restransmit error\n");
                continue;
            }
            log_info("package: 4th restransmit package\n");
            log_info("src_id:%d, dst_id:%d, package_id:%d\n",
                        info.recv_package_push.src_id,
                        info.recv_package_push.dst_id,
                        info.recv_package_push.package_id);

        }else if(info.type == APP_GET_PACKAGE_ACK){
            log_info("package: 5th get recevier finished ACK\n");
            uint32_t package_id = info.recv_package_ack.package_id;
            app_package_t  *package_info = find_package_info(package_id);
            package_info->state = FINISHED;
            if(package_info == NULL){
                log_error("can not find package_id:%d\n", package_id);
                continue;
            }
            //TODO: add info printf
            tank_id_t index =  find_id_index(package_info->src_id);
            if(tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &info, TANK_MSG_NORMAL_SIZE) == TANK_FAIL){
                log_error("package: 4th restransmit error\n");
                continue;
            }
            // tank_mm_free(&g_inner_service_mm, (void*)(package_info->addr_shift + g_shm_base));
            log_info("=====package transmit exit======\n");
        }else if(info.type == HEART_BEAT){
            int index = find_id_index(info.heart_beat.src_id);
            if(index < 0){
                log_error("can not find id:%d\n", info.heart_beat.src_id);
                continue;
            }
            get_cur_time(&app_info_table[index].last_refresh);
            log_debug("heart beat ACK, src_id:%d\n", info.heart_beat.src_id);
        }else if(info.type == APP_READ_PACKAGE_FINISHED){
            log_info("package: 6th read package finished ACK\n");
            uint32_t package_id = info.recv_package_finished.package_id;
            app_package_t  *package_info = find_package_info(package_id);
            if(package_info == NULL){
                log_error("can not find package_id:%d\n", package_id);
                continue;
            }
            tank_mm_free(&g_inner_service_mm, (void*)(package_info->addr_shift + g_shm_base));
            log_info("=====package: 6th read package finished ACK exit======\n");
        }else{
            log_error("inner_service get a error msg type, %d\n", info.type);
        }
    }
    return NULL;
}





int main(int argc, char *argv[])
{
    pthread_t pid;
    tank_log_init(&mylog, "inner",2048, LEVEL_INFO,
                LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                PORT_FILE|PORT_SHELL
                );
    log_info("========logger start===========\n");

    inner_service_init();
    pthread_create(&pid, NULL, &hear_beat_send_thread, NULL);

    // while(1){
    //     app_info_t test;
    //     get_cur_time(&test);
    //     sleep(1);
    // }
    inner_service_deinit();
    log_info("[innr_service]:ending!\n");
    return 0;
}

// typedef int pority_t;

// tank_status_t package_pority_send(app_package_t package)
// {
//     pority_t pority = 0;
//     pority = get_pority(package);
//     tank_msgq_pority(package, pority);
//     return TANK_SUCCESS;
// }