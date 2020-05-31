#include "tank_app.h"
#include "tank_app_pub.h"
#include "tank_msgq.h"
#include "tank_map.h"
#include "tank_request.h"
#include "tank_ID.h"
#include "tcp_fsm.h"
#include "tank_delay.h"
#include <pthread.h>

#include "tank_log_api.h"
#define FILE_NAME "tank_app"

#define APP_HEAP_SIZE (4*1024)
static tank_status_t app_allocate_heap(ta_info_t *ta);
static tank_status_t app_allocate_msgq(ta_info_t *ta);

uint8_t g_shm_baseet_flag_s = 0;
tank_msgq_t *g_service_msgq = NULL;




static tank_status_t find_tcp_state_index(ta_info_t *ta, tank_id_t id, tank_id_t *index);
static tank_status_t creat_ta_id(ta_info_t *ta, tank_id_t id);

void *recv_fun(void *arg)
{
    ta_info_t *ta = (ta_info_t*)arg;
    log_info("[%s] recv_thread running!\n", ta->name);
    my_sem_post(&ta->thread_sem);
    while(1){
        tank_app_recv_all(ta);
    }
}

//TODO: add recv mail check
//TODO: add thread pool
//


void *send_fun(void *arg)
{
    ta_info_t *ta = (ta_info_t*)arg;
    my_sem_wait(&ta->thread_sem);
    log_info("[%s] send_thread running!\n", ta->name);
    while(1){
        app_package_info_t package_info;
        for(int i=0;i<list_get_len(&ta->send_package_status);i++){
            list_get_node(&ta->send_package_status, i, &package_info);
            send_package_state_t state = package_info.send_state;
            tcp_state_t cur_state = 0;
            log_info("dst_id is %d\n", package_info.dst_id);
            find_tcp_state(ta, package_info.dst_id, &cur_state);
            if(cur_state == ESTABLISHED){
                if(state == SEND_WAIT_REQUEST){
                    send_package_request(ta, i);
                }else if(state == SEND_FINISHE){
                    send_package_finished(ta, i);
                }else{
                }
            }else if(cur_state == SYN_SENT){
                log_info("TCP is SYN_SENT\n");
            }else{
                log_warn("TCP not estabilished, wait for established\n");
                write_tcp_state(ta, package_info.dst_id, SYN_SENT);
                tank_app_tcp_send(ta, package_info.dst_id, TCP_SYN);
            }
        }
        sleep_ms(20);
    }
}

tank_status_t tank_app_recv_all(ta_info_t *ta)
{
    tank_id_t src_id;
    app_request_info_t info;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);

    if(info.type == APP_TCP_MSG){
        log_info("[%s][TCP]shake hand, recv a msg\n", ta->name);
        tcp_state_t cur_state = 0;
        tcp_state_t next_state = 0;
        tcp_header_flag_t recv_flag = 0;
        tcp_header_flag_t send_flag = 0;
        recv_flag = info.tcp_state.flag;
        src_id = info.tcp_state.src_id;

        tank_status_t (*fsm_action)(void *) = NULL;

        if(check_ta_id_exist(ta, src_id) == TANK_FAIL){
            log_error("[%s][TCP]ID:%d invalid, check it!\n", ta->name, src_id);
            tank_app_tcp_send(ta, src_id, TCP_ID_INVALID);
            return TANK_FAIL;
        }
        find_tcp_state(ta, src_id, &cur_state);

        if(find_fsm_table(cur_state, &next_state, recv_flag, &send_flag, &fsm_action) ==  TANK_FAIL){
            return TANK_FAIL;
        }
        fsm_action(NULL);
        write_tcp_state(ta, src_id, next_state);
        if(send_flag != TCP_NON){
            tank_app_tcp_send(ta, src_id, send_flag);
        }else{
            log_info("[%s][TCP]unnecessary to send\n", ta->name);
        }
        log_info("[%s][TCP]shake hand, exit\n", ta->name);
    }else if(info.type == APP_GET_PACKAGE_ALLOCATE){
        app_package_info_t package_info;
        for(int i=0;i<list_get_len(&ta->send_package_status);i++){
            list_get_node(&ta->send_package_status, i, &package_info);
            send_package_state_t state = package_info.send_state;
            if(state == SEND_WAIT_ALLOCATE){
                get_package_allocate(ta, i, &info);
                break;
            }else{

            }
        }
    }else if(info.type == APP_recv_package_push){
        tank_id_t src_id = info.recv_package_push.src_id;
        if(check_ta_id_exist(ta, src_id) == TANK_FAIL){
            log_error("[%s]receiver:ID:%d invalid, check it!\n", ta->name, src_id);
            tank_app_tcp_send(ta, src_id, TCP_ID_INVALID);
            return TANK_FAIL;
        }
        tcp_state_t cur_state = 0;
        find_tcp_state(ta, src_id, &cur_state);
        if(cur_state == ESTABLISHED){

            app_package_info_t package_info;
            package_info.src_id = info.recv_package_push.src_id;
            package_info.dst_id = info.recv_package_push.dst_id;
            package_info.size = info.recv_package_push.size;
            package_info.package = (void*)(info.recv_package_push.addr_shift +g_shm_base);
            log_info("[%s] 1st: get a package from src_id:%d, size:%d\n",
                    ta->name, info.recv_package_push.src_id, info.recv_package_push.size);

            if(list_add_node(&ta->recv_package_list, &package_info)==TANK_FAIL){
                //return TANK_FAIL;
            }
            log_info("[%s] 2st: has written into buffer\n",
                    ta->name);
            if(ta->recv_type == RECV_ASYNC){
                if(list_is_full(&ta->recv_package_list)==TANK_SUCCESS){
                    while(list_is_empty(&ta->recv_package_list)==TANK_FAIL){
                        app_package_info_t recv_package_info;
                        list_get_node(&ta->recv_package_list, 0, &recv_package_info);
                        uint32_t num;
                        memcpy(&num, recv_package_info.package, recv_package_info.size);
                        log_info("[inner output]=======get a messgae from src_id:%d, size:%d, info:%d=======\n",
                                recv_package_info.src_id, recv_package_info.size, num);
                        list_delete_node(&ta->recv_package_list, 0);
                    }
                }
                log_info("[%s]RECV ASYNC, client to check\n", ta->name);
            }else if(ta->recv_type == RECV_SYNC){
                ta->recv_package_cb(&package_info);

                app_request_info_t app_info;
                memset(&app_info, 0, TANK_MSGQ_NORMAL_SIZE);
                app_info.type = APP_READ_PACKAGE_FINISHED;
                app_info.recv_package_finished.package_id = package_info.package_id;

                if(tank_msgq_send(ta->sender, &app_info, TANK_MSG_NORMAL_SIZE) == TANK_FAIL){
                    log_error("[%s] send recv_package_finished error\n");
                }
                list_delete_node(&ta->recv_package_list, 0);
                log_info("[%s]RECV SYNC, callback\n", ta->name);
            }else{
                log_error("[%s] the handler recv type error\n", ta->name);
            }
            recv_package_ack(ta, info.recv_package_push.package_id);
        }else{
            log_warn("[%s]receiver: ID:%d TCP-state not established\n", ta->name, src_id);
        }
    }else if(info.type == APP_GET_PACKAGE_ACK){
        app_package_info_t package_info;
        int index = 0;
        for(index=0;index<list_get_len(&ta->send_package_status);index++){
            list_get_node(&ta->send_package_status, index, &package_info);
            if(package_info.package_id == info.recv_package_ack.package_id){
                break;
            }
        }
        package_info.send_state = SEND_IDLE;
        list_rewrite_node(&ta->send_package_status, index, &package_info);
        list_delete_node(&ta->send_package_status, index);
        log_info("[%s]get the package ack successful\n", ta->name);
    }
    else if(info.type == HEART_BEAT){
        app_request_info_t beat_info;
        memset(&beat_info, 0, TANK_MSG_NORMAL_SIZE);
        beat_info.type = HEART_BEAT;
        beat_info.heart_beat.src_id = ta->id;
        beat_info.heart_beat.value = info.heart_beat.value + 1;
        int val = 0;
        my_sem_get_val(&ta->sender->sem_cur_len, &val);
        tank_msgq_send(ta->sender, &beat_info, TANK_MSG_NORMAL_SIZE);
        log_debug("[%s]send the heart beat response\n", ta->name);
    }
    else{
        log_info("[%s]msg type is error\n", ta->name);
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}

tank_status_t find_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t *state)
{
    tank_id_t index = 0;
    find_tcp_state_index(ta, id, &index);
    log_debug("[%s][find_tcp_state]id:%d state:%d\n", ta->name, id, ta->connect_status[index].state);
    *state = ta->connect_status[index].state;
    return TANK_SUCCESS;
}
tank_status_t write_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t state)
{
    tank_id_t index = 0;
    find_tcp_state_index(ta, id, &index);
    log_info("[%s][write_tcp_state]id:%d cur state:%d, next state:%d\n", ta->name, id, ta->connect_status[index].state, state);
    ta->connect_status[index].state = state;
    return TANK_SUCCESS;
}

static tank_status_t creat_ta_id(ta_info_t *ta, tank_id_t id)
{
    if(check_ta_id_exist(ta, id) == TANK_SUCCESS){
        return TANK_FAIL;
    }
    ta->index_lut[ta->id_cur_index].id = id;
    ta->index_lut[ta->id_cur_index].index = ta->id_cur_index;
    ta->id_cur_index += 1;
    return TANK_SUCCESS;
}

tank_status_t tank_app_listen(ta_info_t *ta, tank_id_t id)
{
    creat_ta_id(ta, id);
    write_tcp_state(ta, id, LISTEN);
    return TANK_SUCCESS;
}

tank_status_t tank_app_uart_client(ta_info_t *ta, tank_id_t id)
{
    creat_ta_id(ta, id);
    write_tcp_state(ta, id, LISTEN);
    return TANK_SUCCESS;
}

tank_status_t check_ta_id_exist(ta_info_t *ta, tank_id_t id)
{
    for(int i=0; i<ta->id_cur_index; i++){
        if(ta->index_lut[i].id == id){
            return TANK_SUCCESS;
        }
    }
    return TANK_FAIL;
}
static tank_status_t find_tcp_state_index(ta_info_t *ta, tank_id_t id, tank_id_t *index)
{
    for(int i=0; i<ta->id_cur_index; i++){
        if(ta->index_lut[i].id == id){
            *index = ta->index_lut[i].index;
            log_debug("[%s][find_index]id:%d index:%d\n", ta->name, id, *index);
            return TANK_SUCCESS;
        }
    }
    log_error("[%s][find_index]can not find id:%d\n", ta->name, id);
    return TANK_FAIL;
}
tank_status_t tank_app_recv_cb_register(ta_info_t *ta, tank_status_t (*cb)(app_package_info_t* info))
{

    return TANK_SUCCESS;
}

__weak tank_status_t tank_app_recv_package_callback(app_package_info_t* info)
{

    log_warn("recv callback function not defined\n");
    return TANK_SUCCESS;
}
tank_status_t tank_app_creat(ta_info_t *ta, tank_id_t id, ta_protocol_t protocol, ta_type_t type, recv_type_t recv_type)
{
    log_info("============ app init start ===========\n");
    memset(ta, 0, sizeof(ta_info_t));
    pthread_mutex_init(&ta->thread_mutex, NULL);
    my_sem_creat(&ta->thread_sem, 0);
    ta->recv_type = recv_type;
    ta->recv_thread = recv_fun;
    ta->send_thread = send_fun;

    ta->id_cur_index = 0;
    ta->id = id;
    ta->protocol = protocol;
    ta->type = type;
    ta->send_package_cur_index = 0;
    ta->recv_package_cb = tank_app_recv_package_callback;
    snprintf(ta->name, 32, "APP%d", ta->id);
    app_allocate_heap(ta);
    app_allocate_msgq(ta);
    list_creat(&ta->send_package_status, &ta->mm_handler, sizeof(app_package_info_t), 20);
    list_creat(&ta->recv_package_list, &ta->mm_handler, sizeof(app_package_info_t), 5);
    pthread_create(&ta->pid, NULL, ta->recv_thread, ta);
    pthread_create(&ta->pid, NULL, ta->send_thread, ta);
    log_info("============ app init OK ===========\n");
    return TANK_SUCCESS;
}

tank_status_t tank_app_destory(ta_info_t *ta)
{
    pthread_join(ta->pid, NULL);
    return TANK_FAIL;
}

tank_status_t ta_send_package(ta_info_t *ta, tank_id_t dst_id, void *package, uint32_t size, uint32_t tiemout)
{
    app_package_info_t info;

    info.src_id = ta->id;
    info.dst_id = dst_id;
    info.package = package;
    info.size = size;
    info.send_state = SEND_WAIT_REQUEST;
    if(check_ta_id_exist(ta, dst_id) ==  TANK_FAIL){
        log_info("[%s]dst_id:%d not creat, creat it!\n", ta->name, dst_id);
        tank_app_listen(ta, dst_id);
    }
    if(list_add_node(&ta->send_package_status, &info) == TANK_FAIL){
        log_error("[%s]ta_send_package into buffer error, list is full!\n");
        return TANK_FAIL;
    }
    log_info("[%s]==========start send a package==========\n", ta->name);
    log_info("[%s] pre: write packgae into buffer, src_id:%d, dst_id:%d, size:%d\n",
        ta->name, ta->id, dst_id, size);
    return TANK_SUCCESS;
}
tank_status_t send_package_request(ta_info_t *ta, uint16_t index)
{
    app_request_info_t info;
    app_package_info_t package_info;
    tank_status_t send_status = 0;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    list_get_node(&ta->send_package_status, index, &package_info);
    info.type = APP_SEND_PACKAGE_REQUEST;
    info.send_package_request.src_id = package_info.src_id;
    info.send_package_request.dst_id = package_info.dst_id;
    info.send_package_request.size = package_info.size;

    package_info.send_state = SEND_WAIT_ALLOCATE;
    list_rewrite_node(&ta->send_package_status, index, &package_info);
    log_info("[%s] 1st: send_package_request, src_id:%d, dst_id:%d, size:%d\n",
            ta->name, info.send_package_request.src_id, info.send_package_request.dst_id, info.send_package_request.size
            );
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s][send_package_request]send error\n", ta->name);
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}
tank_status_t get_package_allocate(ta_info_t *ta, uint16_t index, app_request_info_t *info)
{
    addr_t addr = 0;
    app_package_info_t package_info;
    list_get_node(&ta->send_package_status, index, &package_info);
    if(info->recv_package_allocate.dst_id == ta->id){
        addr = info->recv_package_allocate.addr_shift + g_shm_base;
        package_info.package_id = info->recv_package_allocate.package_id;
    }else{
        log_info("[%s][get_package_allocate]recv id is error, get_package_allocate dst id %d\n", ta->name, info->recv_package_allocate.dst_id);
        return TANK_FAIL;
    }
    log_info("[%s] 2nd: get package allocate info, package_id:%d shift:%d\n", ta->name, package_info.package_id, info->recv_package_allocate.addr_shift);
    memcpy((void*)addr, package_info.package, package_info.size);
    package_info.send_state = SEND_FINISHE;
    list_rewrite_node(&ta->send_package_status, index, &package_info);
    return TANK_SUCCESS;
}
tank_status_t send_package_finished(ta_info_t *ta, uint16_t index)
{
    app_request_info_t info;
    tank_status_t send_status = 0;
    app_package_info_t package_info;
    list_get_node(&ta->send_package_status, index, &package_info);
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_SEND_PACKAGE_FINISHED;
    info.send_package_finished.package_id = package_info.package_id;
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s][send_package_finished ACK]send error\n", ta->name);
        return TANK_FAIL;
    }
    package_info.send_state = RECV_ACK;
    list_rewrite_node(&ta->send_package_status, index, &package_info);
    // package_info.state = SEND_IDLE;
    // list_delete_node(&ta->send_package_status, index);
    // list_rewrite_node(&ta->send_package_status, index, &package_info);

    log_info("[%s] 3rd: send package finished, ACK, package_id:%d\n",
            ta->name, info.send_package_finished.package_id
            );
    return TANK_SUCCESS;
}
tank_status_t recv_package_ack(ta_info_t *ta, uint32_t package_id)
{
    app_request_info_t info;
    tank_status_t send_status = 0;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_GET_PACKAGE_ACK;
    info.recv_package_ack.package_id = package_id;
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s][recv_package_ack]send error\n", ta->name);
    }
    log_info("[%s] 2nd: send a ACK, package_id:%d\n",
            ta->name, info.send_package_finished.package_id
            );
    return TANK_SUCCESS;
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

tank_status_t tank_app_tcp_send(ta_info_t *ta, tank_id_t dst_id, tcp_header_flag_t flag)
{
    app_request_info_t info;
    if(ta->id == dst_id){
        return TANK_FAIL;
    }
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_TCP_MSG;
    info.tcp_state.src_id = ta->id;
    info.tcp_state.dst_id = dst_id;
    info.tcp_state.flag = flag;
    tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s][TCP]send a msg, src_id:%d, dst_id:%d, flag:%d\n",
            ta->name, ta->id, dst_id, flag
            );
    return TANK_SUCCESS;
}

// tank_status_t tank_app_recv_msg(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag)
// {
//     app_request_info_t info;
//     memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
//     if(tank_msgq_recv(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE) == TANK_FAIL){
//         return TANK_FAIL;
//     }
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


tank_status_t tank_app_recv_msg_wait(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag)
{
    app_request_info_t info;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);
    if(info.type == APP_TCP_MSG){
        if(info.tcp_state.dst_id == ta->id){
            *flag = info.tcp_state.flag;
            *src_id = info.tcp_state.src_id;
        }else{
            log_info("[%s]recv id is error, msg dst id %d\n", ta->name, info.tcp_state.dst_id);
            return TANK_FAIL;
        }
    }else{
        log_info("[%s]recv type is error\n", ta->name);
        return TANK_FAIL;
    }
    log_info("[%s]recv a msg, src_id:%d, dst_id:%d, flag:%d\n",
            ta->name, *src_id, ta->id, *flag
            );
    return TANK_SUCCESS;
}


static tank_status_t app_allocate_msgq(ta_info_t *ta)
{
    uint16_t mm_size = sizeof(tank_msgq_t)+TANK_MSGQ_NORMAL_LEN*TANK_MSGQ_NORMAL_SIZE;

    ta->receiver = (tank_msgq_t*)tank_mm_malloc(&ta->mm_handler, mm_size);
    if(ta->receiver == NULL){
        log_error("ta->receiver allocate memory fail, heap full\n");
    }
    tank_msgq_creat(ta->receiver, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    log_info("[%s]receiver msgq addr:%p\n", ta->name, ta->receiver);

    ta->sender = g_service_msgq;
    log_info("[%s]sender msgq addr:%p\n", ta->name, ta->sender);

    log_info("[%s]heap size remain:%d\n", ta->mm_handler.name, ta->mm_handler.heap.xFreeBytesRemaining);

    app_request_info_t app_info;
    memset(&app_info, 0, TANK_MSGQ_NORMAL_SIZE);

    app_info.type = APP_PUSH_MSGQ_ADDR;
    app_info.msgq.id = ta->id;
    app_info.msgq.recv_shift = (uint32_t)ta->receiver - g_shm_base;
    app_info.msgq.send_shift = (uint32_t)ta->sender - g_shm_base;

    tank_msgq_send(ta->sender, &app_info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s]send msgq info to inner_service\n", ta->name);
    log_info("[%s]id:%d, type:%d, recv_shift:%d, send_shift:%d\n", ta->name, app_info.msgq.id, app_info.type, app_info.msgq.recv_shift, app_info.msgq.send_shift);

    return TANK_SUCCESS;
}

static tank_status_t app_allocate_heap(ta_info_t *ta)
{
    app_request_info_t info;
    if(g_shm_baseet_flag_s == 0){
        get_shm_base_addr();
        g_shm_baseet_flag_s = 1;
    }
    get_service_msgq_addr(&g_service_msgq);

    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = MM_ALLOCATE;
    info.heap.id = ta->id;
    info.heap.size = APP_HEAP_SIZE;
    log_info("[%s]send allocate heap request, id:%d, len:%d\n", ta->name, info.heap.id, info.heap.size);

    tank_msgq_send(g_service_msgq, &info, TANK_MSGQ_NORMAL_SIZE);

    log_info("[%s]wait for inner_service allocate finished\n", ta->name);
    inner_service_push_heap_t *app_get = (inner_service_push_heap_t *)(SEM_ADDR);
    my_sem_wait(&app_get->sem);
    log_info("[%s]inner_service allocate OK\n", ta->name);
    log_info("[%s]allocate info, addr_base:%p, shift:%d\n", ta->name, (void*)g_shm_base, app_get->shift);

    char name[48] = {0};
    snprintf(name, 48, "%s_mm", ta->name);
    tank_mm_register(&ta->mm_handler, g_shm_base+app_get->shift , APP_HEAP_SIZE, name);

    return TANK_SUCCESS;
}

