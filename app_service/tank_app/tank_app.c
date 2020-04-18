#include "tank_app.h"
#include "tank_app_pub.h"
#include "tank_msgq.h"
#include "tank_map.h"
#include "tank_request.h"
#include "tank_ID.h"
#include "tcp_fsm.h"
#include "tank_delay.h"
#include <pthread.h>




#define APP_HEAP_SIZE 1024
static tank_status_t app_allocate_heap(ta_info_t *ta);
static tank_status_t app_allocate_msgq(ta_info_t *ta);

uint8_t g_shm_baseet_flag_s = 0;
tank_msgq_t *g_service_msgq = NULL;



#include "tank_log_api.h"
#define FILE_NAME "tank_app"

static tank_status_t find_tcp_state_index(ta_info_t *ta, tank_id_t id, tank_id_t *index);
static tank_status_t creat_ta_id(ta_info_t *ta, tank_id_t id);

void *recv_fun(void *arg)
{
    ta_info_t *ta = (ta_info_t*)arg;
    log_info("[%s] recv_thread running!\n", ta->name);
    my_sem_post(&ta->thread_sem);
    static uint32_t fsm_cnt = 0;
    while(1){
        tank_app_recv_all(ta);
    }
}


void *send_fun(void *arg)
{
    ta_info_t *ta = (ta_info_t*)arg;
    my_sem_wait(&ta->thread_sem);
    log_info("[%s] send_thread running!\n", ta->name);
    static uint32_t send_cnt = 0;
    while(1){
        app_package_info_t package_info;
        for(int i=0;i<list_get_len(&ta->send_package_status);i++){
            list_get_node(&ta->send_package_status, i, &package_info);
            send_package_state_t state = package_info.state;
            tcp_state_t cur_state = 0;
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
        // send_cnt += 1;
        // log_info("send_cnt running cnt:%d\n", send_cnt);
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
        recv_flag = info.msg.flag;
        src_id = info.msg.src_id;

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
            send_package_state_t state = package_info.state;
            if(state == SEND_WAIT_ALLOCATE){
                get_package_allocate(ta, i, &info);
                break;
            }else{

            }
        }
    }else if(info.type == APP_GET_PACKAGE_PUSH){
        tank_id_t src_id = info.get_package_push.src_id;
        if(check_ta_id_exist(ta, src_id) == TANK_FAIL){
            log_error("[%s]receiver:ID:%d invalid, check it!\n", ta->name, src_id);
            tank_app_tcp_send(ta, src_id, TCP_ID_INVALID);
            return TANK_FAIL;
        }
        tcp_state_t cur_state = 0;
        find_tcp_state(ta, src_id, &cur_state);
        if(cur_state == ESTABLISHED){
            tank_msgq_send(ta->recv_package, &info.get_package_push, sizeof(app_get_package_push_t));
            log_info("[%s] 1st: get a package, write into recv msgq buffer\n", ta->name);
        }else{
            log_warn("[%s]receiver: ID:%d TCP-state not established\n", ta->name, src_id);
        }
    }else{
        log_info("[%s]msg type is error\n", ta->name);
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}

tank_status_t ta_recv_package(ta_info_t *ta, tank_id_t *src_id, void* packgae, uint16_t *size, uint16_t oversize)
{
    app_get_package_push_t info;
    memset(&info, 0, sizeof(app_get_package_push_t));
    tank_msgq_recv_wait(ta->recv_package, &info, sizeof(app_get_package_push_t));
    *size = info.size;
    *src_id = info.src_id;
    if(*size > oversize){
        memcpy(packgae, (void*)(info.addr_shift+g_shm_base), oversize);
    }else{
        memcpy(packgae, (void*)(info.addr_shift+g_shm_base), *size);
    }
    log_info("[%s][recv_package_msgq]get a package from src_id:%d, size:%d\n", ta->name, *src_id, *size);
    get_package_finished(ta, info.package_id);
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

tank_status_t tank_app_creat(ta_info_t *ta, tank_id_t id, ta_protocol_t protocol, ta_type_t type)
{
    log_info("============ app init start ===========\n");
    memset(ta, 0, sizeof(ta_info_t));
    pthread_mutex_init(&ta->thread_mutex, NULL);
    my_sem_creat(&ta->thread_sem, 0);
    ta->recv_thread = recv_fun;
    ta->send_thread = send_fun;

    ta->id_cur_index = 0;
    ta->id = id;
    ta->protocol = protocol;
    ta->type = type;
    ta->send_package_cur_index = 0;
    snprintf(ta->name, 32, "APP%d", ta->id);
    app_allocate_heap(ta);
    app_allocate_msgq(ta);
    list_creat(&ta->send_package_status, &ta->mm_handler, sizeof(app_package_info_t), 20);
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
    info.state = SEND_WAIT_REQUEST;

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
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s][send_package_request]send error\n", ta->name);
        return TANK_FAIL;
    }
    package_info.state = SEND_WAIT_ALLOCATE;
    list_rewrite_node(&ta->send_package_status, index, &package_info);
    log_info("[%s] 1st: send_package_request, src_id:%d, dst_id:%d, size:%d\n",
            ta->name, info.send_package_request.src_id, info.send_package_request.dst_id, info.send_package_request.size
            );
    return TANK_SUCCESS;
}
tank_status_t get_package_allocate(ta_info_t *ta, uint16_t index, app_request_info_t *info)
{
    addr_t addr = 0;
    app_package_info_t package_info;
    list_get_node(&ta->send_package_status, index, &package_info);
    if(info->send_package_allocate.dst_id == ta->id){
        addr = info->send_package_allocate.addr_shift + g_shm_base;
        package_info.package_id = info->send_package_allocate.package_id;
    }else{
        log_info("[%s][get_package_allocate]recv id is error, get_package_allocate dst id %d\n", ta->name, info->send_package_allocate.dst_id);
        return TANK_FAIL;
    }
    log_info("[%s] 2nd: get package allocate info, package_id:%d shift:%d\n", ta->name, package_info.package_id, info->send_package_allocate.addr_shift);
    memcpy((void*)addr, package_info.package, package_info.size);
    package_info.state = SEND_FINISHE;
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
    info.send_package_finshed.package_id = package_info.package_id;
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s][send_package_finished ACK]send error\n", ta->name);
        return TANK_FAIL;
    }
    package_info.state = SEND_IDLE;
    list_delete_node(&ta->send_package_status, index);
    // list_rewrite_node(&ta->send_package_status, index, &package_info);

    log_info("[%s] 3rd: send package finished, ACK, package_id:%d\n",
            ta->name, info.send_package_finshed.package_id
            );
    return TANK_SUCCESS;
}
tank_status_t get_package_finished(ta_info_t *ta, uint32_t package_id)
{
    app_request_info_t info;
    tank_status_t send_status = 0;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_GET_PACKAGE_ACK;
    info.get_package_finished.package_id = package_id;
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s][get_package_finished]send error\n", ta->name);
    }
    log_info("[%s] 2nd: send a ACK, package_id:%d\n",
            ta->name, info.send_package_finshed.package_id
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

    if(info.type == APP_GET_PACKAGE_PUSH){
        if(info.get_package_push.dst_id == ta->id){
            *src_id = info.get_package_push.src_id;
            package_id = info.get_package_push.package_id;
            *size = info.get_package_push.size;
            addr_shift = info.get_package_push.addr_shift;
            addr = info.get_package_push.addr_shift + g_shm_base;
        }else{
            log_info("[%s]recv id is error, tank_app_recv_package_wait dst id %d\n", ta->name, info.get_package_push.dst_id);
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
    info.get_package_finished.package_id = package_id;
    tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s]get_package_finished, package_id:%d\n", ta->name, package_id);
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
    info.msg.src_id = ta->id;
    info.msg.dst_id = dst_id;
    info.msg.flag = flag;
    tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s][TCP]send a msg, src_id:%d, dst_id:%d, flag:%d\n",
            ta->name, ta->id, dst_id, flag
            );
    return TANK_SUCCESS;
}

tank_status_t tank_app_recv_msg(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag)
{
    app_request_info_t info;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    if(tank_msgq_recv(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE) == TANK_FAIL){
        return TANK_FAIL;
    }
    if(info.type == APP_TCP_MSG){
        if(info.msg.dst_id == ta->id){
            *flag = info.msg.flag;
            *src_id = info.msg.src_id;
        }else{
            log_info("[%s]recv id is error, msg dst id %d\n", ta->name, info.msg.dst_id);
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


tank_status_t tank_app_recv_msg_wait(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag)
{
    app_request_info_t info;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);
    if(info.type == APP_TCP_MSG){
        if(info.msg.dst_id == ta->id){
            *flag = info.msg.flag;
            *src_id = info.msg.src_id;
        }else{
            log_info("[%s]recv id is error, msg dst id %d\n", ta->name, info.msg.dst_id);
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

    mm_size = sizeof(tank_msgq_t) + 10*sizeof(app_get_package_push_t);
    ta->recv_package = (tank_msgq_t*)tank_mm_malloc(&ta->mm_handler, mm_size);
    if(ta->recv_package == NULL){
        log_error("ta->recv_package allocate memory fail, heap full\n");
    }
    tank_msgq_creat(ta->recv_package, sizeof(app_get_package_push_t), 10);
    log_info("[%s]recv_package msgq addr:%p\n", ta->name, ta->recv_package);

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

