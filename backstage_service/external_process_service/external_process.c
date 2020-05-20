#include "external_process.h"

#include "tank_msgq.h"
#include "tank_map.h"
#include "tank_request.h"
#include "tank_ID.h"
#include "tank_delay.h"
#include <pthread.h>

#include "tank_log_api.h"
#define FILE_NAME "external_process"

#define EXTERNAL_PROCESS_HEAP_SIZE (4*1024)

static tank_status_t external_process_recv_all(external_process_info_t *ta);
static tank_status_t external_process_allocate_heap(external_process_info_t *handler);
static tank_status_t external_process_allocate_msgq(external_process_info_t *handler);

uint8_t g_shm_baseet_flag_s = 0;
tank_msgq_t *g_service_msgq = NULL;


void *external_process_recv_thread(void *arg)
{
    external_process_info_t *handler = (external_process_info_t*)arg;
    log_info("external_process_recv_thread running!\n");
    my_sem_post(&handler->thread_sem);
    while(1){
        external_process_recv_all(handler);
        // tank_app_recv_all(ta);
        // log_info("recv_thread running!\n");
        // sleep_ms(1000);
    }
}

void *external_process_send_thread(void *arg)
{
    external_process_info_t *handler = (external_process_info_t*)arg;
    my_sem_wait(&handler->thread_sem);
    log_info("send_thread running!\n");
    while(1){
        // TODO:
        if(list_is_full(&handler->recv_package_list)==TANK_SUCCESS){
            while(list_is_empty(&handler->recv_package_list)==TANK_FAIL){
                app_package_info_t recv_package_info;
                list_get_node(&ta->recv_package_list, 0, &recv_package_info);
                uint32_t num;
                memcpy(&num, recv_package_info.package, recv_package_info.size);
                log_info("[inner output]=======get a messgae from src_id:%d, size:%d, info:%d=======\n",
                        recv_package_info.src_id, recv_package_info.size, num);
                list_delete_node(&ta->recv_package_list, 0);
            }
        }
        log_info("[%s]RECV ASYNC, clientto check\n", ta->name);
        log_info("send_thread running!\n");
        sleep_ms(1000);
    }
}

static tank_status_t external_process_recv_all(external_process_info_t *ta)
{
    app_request_info_t info;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);
    if(info.type == HEART_BEAT){
        app_request_info_t beat_info;
        memset(&beat_info, 0, TANK_MSG_NORMAL_SIZE);
        beat_info.type = HEART_BEAT;
        beat_info.heart_beat.src_id = ta->id;
        beat_info.heart_beat.value = info.heart_beat.value + 1;
        int val = 0;
        my_sem_get_val(&ta->sender->sem_cur_len, &val);
        tank_msgq_send(ta->sender, &beat_info, TANK_MSG_NORMAL_SIZE);
        log_debug("send the heart beat response\n");
    }else if(info.type > HEART_BEAT){
        // TODO: retransmit
        list_add_node(&ta->recv_package_list, &info);
    }else{
        log_error("msg type is error\n");
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}

tank_status_t tank_app_recv_cb_register(ta_info_t *ta, tank_status_t (*cb)(app_package_info_t* info))
{
    return TANK_SUCCESS;
}

__weak tank_status_t external_process_recv_package_callback(app_package_info_t* info)
{

    log_warn("external_process_recv_package_callback function not defined\n");
    return TANK_SUCCESS;
}

tank_status_t external_service_init(external_process_info_t *handler)
{
    log_info("============ external init start ===========\n");
    memset(handler, 0, sizeof(external_process_info_t));
    pthread_mutex_init(&handler->thread_mutex, NULL);
    my_sem_creat(&handler->thread_sem, 0);

    handler->recv_thread = external_process_recv_thread;
    handler->send_thread = external_process_send_thread;

    handler->id = 0;

    handler->recv_package_cb = external_process_recv_package_callback;
    snprintf(handler->name, 32, "external_process");
    external_process_allocate_heap(handler);
    external_process_allocate_msgq(handler);
    list_creat(&handler->send_package_list, &handler->mm_handler, sizeof(app_package_info_t), 20);
    list_creat(&handler->recv_package_list, &handler->mm_handler, sizeof(app_package_info_t), 20);
    pthread_create(&handler->pid, NULL, handler->recv_thread, handler);
    pthread_create(&handler->pid, NULL, handler->send_thread, handler);
    log_info("============ app init OK ===========\n");
    return TANK_SUCCESS;
}

tank_status_t tank_app_destory(ta_info_t *ta)
{
    pthread_join(ta->pid, NULL);
    return TANK_FAIL;
}


tank_status_t tank_app_recv_package_wait(ta_info_t *ta, tank_id_t *src_id, void *package, uint32_t *size, uint32_t max_size)
{
    app_request_info_t info;
    uint32_t package_id;
    addr_t addr;
    addr_shift_t addr_shift;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);

    if(info.type == APP_recv_package_push){
        if(info.recv_package_push.dst_id == ta->id){
            *src_id = info.recv_package_push.src_id;
            package_id = info.recv_package_push.package_id;
            *size = info.recv_package_push.size;
            addr_shift = info.recv_package_push.addr_shift;
            addr = info.recv_package_push.addr_shift + g_shm_base;
        }else{
            log_info("[%s]recv id is error, tank_app_recv_package_wait dst id %d\n", ta->name, info.recv_package_push.dst_id);
            return TANK_FAIL;
        }
    }else{
        log_info("[%s]recv type is error\n", ta->name);
        return TANK_FAIL;
    }
    log_info("[%s]tank_app_recv_package_wait, src_id:%d, dst_id:%d, package_id:%d, size:%d, shift:%d\n",
    ta->name, *src_id, ta->id, package_id, *size, addr_shift);

    memcpy(package, (void*)addr, *size);

    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_GET_PACKAGE_ACK;
    info.recv_package_ack.package_id = package_id;
    tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s]recv_package_ack, package_id:%d\n", ta->name, package_id);
    return TANK_SUCCESS;
}



// tank_status_t tank_app_recv_msg_wait(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag)
// {
//     app_request_info_t info;
//     memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
//     tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);
//     if(info.type == APP_TCP_MSG){
//         if(info.msg.dst_id == ta->id){
//             *flag = info.msg.flag;
//             *src_id = info.msg.src_id;
//         }else{
//             log_info("[%s]recv id is error, msg dst id %d\n", ta->name, info.msg.dst_id);
//             return TANK_FAIL;
//         }
//     }else{
//         log_info("[%s]recv type is error\n", ta->name);
//         return TANK_FAIL;
//     }
//     log_info("[%s]recv a msg, src_id:%d, dst_id:%d, flag:%d\n",
//             ta->name, *src_id, ta->id, *flag
//             );
//     return TANK_SUCCESS;
// }


static tank_status_t external_process_allocate_msgq(external_process_info_t *handler)
{
    uint16_t mm_size = sizeof(tank_msgq_t) + 10*20;

    handler->receiver = (tank_msgq_t*)tank_mm_malloc(&handler->mm_handler, mm_size);
    if(handler->receiver == NULL){
        log_error("ta->receiver allocate memory fail, heap full\n");
    }
    tank_msgq_creat(handler->receiver, 20, 10);
    log_info("receiver msgq addr:%p\n", handler->receiver);


    handler->sender = g_service_msgq;
    log_info("sender msgq addr:%p\n", handler->sender);

    log_info("[%s]heap size remain:%d\n", handler->mm_handler.name, handler->mm_handler.heap.xFreeBytesRemaining);

    app_request_info_t app_info;
    memset(&app_info, 0, TANK_MSGQ_NORMAL_SIZE);

    app_info.type = APP_PUSH_MSGQ_ADDR;
    app_info.msgq.id = handler->id;
    app_info.msgq.recv_shift = (uint32_t)handler->receiver - g_shm_base;
    app_info.msgq.send_shift = (uint32_t)handler->sender - g_shm_base;

    tank_msgq_send(handler->sender, &app_info, TANK_MSGQ_NORMAL_SIZE);
    log_info("send msgq info to inner_service\n");
    log_info("id:%d, type:%d, recv_shift:%d, send_shift:%d\n", app_info.msgq.id, app_info.type, app_info.msgq.recv_shift, app_info.msgq.send_shift);

    return TANK_SUCCESS;
}

static tank_status_t external_process_allocate_heap(external_process_info_t *handler)
{
    app_request_info_t info;
    if(g_shm_baseet_flag_s == 0){
        get_shm_base_addr();
        g_shm_baseet_flag_s = 1;
    }
    get_service_msgq_addr(&g_service_msgq);

    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = MM_ALLOCATE;
    info.heap.id = handler->id;
    info.heap.size = EXTERNAL_PROCESS_HEAP_SIZE;
    log_info("send allocate heap request, id:%d, len:%d\n", info.heap.id, info.heap.size);

    tank_msgq_send(g_service_msgq, &info, TANK_MSGQ_NORMAL_SIZE);

    log_info("wait for inner_service allocate finished\n");
    inner_service_push_heap_t *app_get = (inner_service_push_heap_t *)(SEM_ADDR);
    my_sem_wait(&app_get->sem);
    log_info("inner_service allocate OK\n");
    log_info("allocate info, addr_base:%p, shift:%d\n", (void*)g_shm_base, app_get->shift);

    char name[48] = {0};
    snprintf(name, 48, "%s_mm", handler->name);
    tank_mm_register(&handler->mm_handler, g_shm_base+app_get->shift , EXTERNAL_PROCESS_HEAP_SIZE, name);

    return TANK_SUCCESS;
}


tank_status_t phy_link_slect(app_package_t *package)
{
    phy_link_t phy_link;
    phy_link = get_phy_link(package);
    tank_hal_send(phy_link, package);
    return TANK_SUCCESS;
}

phy_link_t get_phy_link(app_package_t *package)
{
    phy_link_id_t id = 0;
    phy_link_t link_type = 0;
    id = parse_package_phy_link(package);
    link_type = parse_json_table(id);
    return link_type;
}
hal_send_status_t tank_hal_send(phy_link_t *phy_link, app_package_t *package)
{
    hal_send_status_t status;
    hal_send_t *sender = get_sender(phy_link);
    status =  hal_send_package(sender, package);
    return status;
}