#include "tank_map.h"
#include "tank_mm.h"
#include "tank_pub.h"
#include "my_sem.h"
#include "tank_msgq.h"
#include <pthread.h>
#include "tank_request.h"
#include "inner_service.h"

app_info_t               app_info_table[256];
tank_id_t                app_id_lut[10] = {0,1,2,3,4,5,6,7,8,9};

uint16_t                 g_app_id_seq = 0;
tank_mm_t                g_in_swap_mm;

pthread_t                g_service_pid;
tank_msgq_t             *g_service_request_msgq;
app_heap_get_t          *g_app_get;
app_request_info_t               g_app_request;

app_package_t app_package_info[100] = {0};
uint16_t       g_app_package_seq = 0;


#include "tank_log_api.h"
#define FILE_NAME "inner_service"


tank_status_t app_malloc_init(void);


app_package_t* find_package_info(tank_id_t package_id)
{
    for(int i=0;i<g_app_package_seq;++i){
        if(app_package_info[i].package_id == package_id){
            log_debug("find package_id id:%d\n", package_id);
            return &app_package_info[i];
        }
    }
    return NULL;
}

void print_app_info_table(void)
{
    log_info("=========app_info_table============\n");
    for(int i=0;i<g_app_id_seq;++i){
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
    tank_mm_register(&g_in_swap_mm, INNER_SWAP_ADDR, INNER_SWAP_SIZE, "inner_service_mm");
    app_malloc_init();
    return TANK_SUCCESS;

}
tank_status_t app_malloc_init(void)
{
    g_app_get = (app_heap_get_t *)SEM_ADDR;
    log_info("sem addr:%p  size:%d\n", &g_app_get->sem, sizeof(my_sem_t));

    sem_destroy(&g_app_get->sem);
    my_sem_creat(&g_app_get->sem, 0);

    g_service_request_msgq = (tank_msgq_t*)tank_mm_malloc(&g_in_swap_mm, sizeof(tank_msgq_t)+50*20);
    log_info("msgq addr:%p\n", g_service_request_msgq);

    *(uint32_t*)MSGQ_MAP_ADDR = (uint32_t)g_service_request_msgq - g_shm_base;
    tank_msgq_creat(g_service_request_msgq, 20, 50);
    log_info("========inner service init OK===========\n");
    return TANK_SUCCESS;
}

int find_id_index(tank_id_t id)
{
    for(int i=0;i<g_app_id_seq;++i){
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



void *main_thread(void *arg)
{
    app_request_info_t info;
    while(1){
        memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
        tank_msgq_recv_wait(g_service_request_msgq, &info, TANK_MSGQ_NORMAL_SIZE);
        if(info.type == MM_ALLOCATE){
            log_info("======app allocate start======\n");
            log_info("id:%d, size:%d\n", info.heap.id, info.heap.size);
            int index = find_id_index(info.heap.id);
            if(index >= 0){
                log_error("id:%d has been malloc\n", info.heap.id);
                continue;
            }
            log_info("start malloc\n");
            void *addr = tank_mm_alloc(&g_in_swap_mm, info.heap.size);
            log_info("[%s]remain:%d\n", g_in_swap_mm.name, g_in_swap_mm.heap.xFreeBytesRemaining);
            uint32_t shift = (uint32_t)addr - g_shm_base;

            app_info_table[g_app_id_seq].id = info.heap.id;
            app_info_table[g_app_id_seq].heap_addr = (void*)addr;

            log_info("id:%d, shift:%d\n", info.heap.id, shift);
            g_app_get->shift = shift;
            g_app_id_seq += 1;

            log_info("======app allocate exit======\n");
            my_sem_post(&g_app_get->sem);
        }else if(info.type == APP_PUSH_MSGQ_ADDR){
            log_info("======app push msgq start======\n");
            log_info("id:%d, type:%d, recv_shift:%d, send_shift:%d\n", info.msgq.id, info.type, info.msgq.recv_shift, info.msgq.send_shift);
            int index = find_id_index(info.msgq.id);
            if(index < 0){
                log_error("can not find id:%d\n", info.msgq.id);
                continue;
            }
            app_info_table[index].msgq_recv_addr = (void*)info.msgq.recv_shift + g_shm_base;
            app_info_table[index].msgq_send_addr = (void*)info.msgq.send_shift + g_shm_base;
            log_info("has write into table\n");
            log_info("======app push msgq exit======\n");
            print_app_info_table();

        }else if(info.type == APP_SEND_MSG){
            log_info("======app msg transmit start======\n");
            int index = find_id_index(info.msg.dst_id);
            if(index < 0){
                log_error("can not find id:%d\n", info.msg.dst_id);
                continue;
            }
            tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &info, APP_MSG_SIZE);
            log_info("src_id:%d, dst_id:%d, flag:%d\n",
                    info.msg.src_id, info.msg.dst_id, info.msg.flag
                    );
            log_info("======app msg transmit exit======\n");
        }else if(info.type == APP_SEND_PACKAGE_REQUEST){
            log_info("======APP_SEND_PACKAGE_REQUEST start======\n");
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
            log_info("start malloc package\n");
            void *addr = tank_mm_alloc(&g_in_swap_mm, info.send_package_request.size);
            uint32_t add_shift = (uint32_t)addr - g_shm_base;
            app_package_info[g_app_package_seq].addr_shift = add_shift;

            log_info("ALLOCATE INFO:src_id:%d, dst_id:%d, package_id:%d, size:%d, addr_shift:%d\n",
                    app_package_info[g_app_package_seq].src_id, app_package_info[g_app_package_seq].dst_id,
                    app_package_info[g_app_package_seq].package_id, app_package_info[g_app_package_seq].size,
                    app_package_info[g_app_package_seq].addr_shift
                    );
            log_info("======APP_SEND_PACKAGE_REQUEST exit======\n");

            memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
            log_info("======APP_GET_PACKAGE_ALLOCATE start======\n");
            index =  find_id_index(app_package_info[g_app_package_seq].src_id);
            info.type = APP_GET_PACKAGE_ALLOCATE;
            info.send_package_allocate.addr_shift = add_shift;
            info.send_package_allocate.dst_id = app_package_info[g_app_package_seq].src_id;
            info.send_package_allocate.package_id = app_package_info[g_app_package_seq].package_id;
            tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &info, TANK_MSG_NORMAL_SIZE);

            log_info("======APP_GET_PACKAGE_ALLOCATE exit======\n");

            g_app_package_seq += 1;
            g_package_id += 1;
        }else if(info.type == APP_SEND_PACKAGE_FINISHED){
            log_info("======APP_SEND_PACKAGE_FINISHED start======\n");
            uint32_t package_id = info.send_package_finshed.package_id;
            app_package_t  *package_info = find_package_info(package_id);
            if(package_info == NULL){
                log_error("can not find package_id:%d\n", package_id);
                continue;
            }
            log_info("ALLOCATE INFO:src_id:%d, dst_id:%d, package_id:%d, size:%d, addr_shift:%d\n",
                    package_info->src_id, package_info->dst_id,
                    package_info->package_id, package_info->size,
                    package_info->addr_shift
                    );

            void *buf = (void*)(package_info->addr_shift + g_shm_base);
            log_info("recv msg:%s\n", buf);
            log_info("======APP_SEND_PACKAGE_FINISHED exit======\n");

            log_info("======APP_GET_PACKAGE_PUSH start======\n");

            memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
            info.type = APP_GET_PACKAGE_PUSH;
            info.get_package_push.src_id = package_info->src_id;
            info.get_package_push.dst_id = package_info->dst_id;
            info.get_package_push.package_id = package_info->package_id;
            info.get_package_push.addr_shift = package_info->addr_shift;
            info.get_package_push.size = package_info->size;
            tank_id_t index =  find_id_index(info.get_package_push.dst_id);
            tank_msgq_send((tank_msgq_t*)app_info_table[index].msgq_recv_addr, &info, TANK_MSG_NORMAL_SIZE);
            log_info("======APP_GET_PACKAGE_PUSH exit======\n");
        }else if(info.type == APP_GET_PACKAGE_FINISHED){
            log_info("======APP_GET_PACKAGE_FINISHED start======\n");
            uint32_t package_id = info.get_package_finished.package_id;
            app_package_t  *package_info = find_package_info(package_id);
            if(package_info == NULL){
                log_error("can not find package_id:%d\n", package_id);
                continue;
            }
            tank_mm_free(&g_in_swap_mm, (void*)(package_info->addr_shift + g_shm_base));
            log_info("======APP_GET_PACKAGE_FINISHED exit======\n");
        }else{
            log_error("error type, %d\n", info.type);
        }
    }
    return NULL;
}





int main(int argc, char *argv[])
{
    tank_log_init(&mylog, "inner",2048, LEVEL_DEBUG,
                LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
                PORT_FILE|PORT_SHELL
                );
    log_info("========logger start===========\n");
    printf("app_request_info_t size:%d\n", sizeof(app_request_info_t));
    inner_service_init();
    pthread_create(&g_service_pid,NULL,&main_thread,NULL);
    pthread_join(g_service_pid,NULL);
    log_info("[inner_service]:ending!\n");
    return 0;
}
