#include "tank_app.h"
#include "tank_app_pub.h"
#include "tank_msgq.h"
#include "tank_map.h"
#include "tank_request.h"
#include "tank_ID.h"
#include "tcp_fsm.h"
#include "tank_delay.h"
#define APP_HEAP_SIZE 1024
static tank_status_t app_allocate_heap(ta_info_t *ta);
static tank_status_t app_allocate_msgq(ta_info_t *ta);

uint8_t g_shm_baseet_flag_s = 0;
tank_msgq_t *g_service_msgq = NULL;
app_request_info_t info;


#include "tank_log_api.h"
#define FILE_NAME "tank_app"

static tank_status_t find_tcp_state_index(ta_info_t *ta, tank_id_t id, tank_id_t *index);
static tank_status_t creat_ta_id(ta_info_t *ta, tank_id_t id);

void *recv_thread(ta_info_t *ta)
{
    static uint32_t fsm_cnt = 0;
    while(1){
        fsm_cnt ++;
        log_info("fsm running cnt:%d\n",fsm_cnt);
        tank_app_recv_all(ta);
        log_info("fsm exit\n");
    }
}


void *send_package_thread(ta_info_t *ta)
{
    static uint32_t send_cnt = 0;
    while(1){
        // printf("send_package_cur_index:%d\n", ta->send_package_cur_index);
        for(int i=0;i<ta->send_package_cur_index;i++){
            send_package_state_t state = ta->send_package_status[i].state;
            if(state == SEND_WAIT_REQUEST){
                send_package_request(ta, i);
            }else if(state == SEND_FINISHE){
                send_package_finished(ta, i);
            }else{

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
    log_info("recv a message\n");
    if(info.type == APP_SEND_MSG){
        tcp_state_t cur_state = 0;
        tcp_state_t next_state = 0;
        tcp_header_flag_t recv_flag = 0;
        tcp_header_flag_t send_flag = 0;
        recv_flag = info.msg.flag;
        src_id = info.msg.src_id;

        tank_status_t (*fsm_action)(void *) = NULL;

        if(check_ta_id_exist(ta, src_id) == TANK_FAIL){
            log_error("ID:%d invalid, check it!\n", src_id);
            tank_app_send_msg(ta, src_id, TCP_ID_INVALID);
            return TANK_FAIL;
        }
        find_tcp_state(ta, src_id, &cur_state);

        if(find_fsm_table(cur_state, &next_state, recv_flag, &send_flag, &fsm_action)){
            return TANK_FAIL;
        }
        fsm_action(NULL);
        write_tcp_state(ta, src_id, next_state);
        if(send_flag != TCP_NON){
            tank_app_send_msg(ta, src_id, send_flag);
        }else{
            log_info("unnecessary to send\n");
        }
    }else if(info.type == APP_GET_PACKAGE_ALLOCATE){
        for(int i=0;i<ta->send_package_cur_index;i++){
            send_package_state_t state = ta->send_package_status[i].state;
            if(state == SEND_WAIT_ALLOCATE){
                get_package_allocate(ta, i, &info);
            }else{

            }
        }
    }else if(info.type == APP_GET_PACKAGE_PUSH){
        tank_msgq_send(ta->recv_package, &info.get_package_push, sizeof(app_get_package_push_t));
        log_info("[%s]APP_GET_PACKAGE_PUSH has write into msgq, recv it\n", ta->name);
    }else{
        log_info("[%s]recv type is error\n", ta->name);
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
    log_info("shift_addr:0x%x\n", info.addr_shift);
    get_package_finished(ta, info.package_id);
    return TANK_SUCCESS;
}

tank_status_t find_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t *state)
{
    tank_id_t index = 0;
    find_tcp_state_index(ta, id, &index);
    log_debug("[find_tcp_state]id:%d state:%d\n", id, ta->connect_status[index].state);
    *state = ta->connect_status[index].state;
    return TANK_SUCCESS;
}
tank_status_t write_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t state)
{
    tank_id_t index = 0;
    find_tcp_state_index(ta, id, &index);
    log_info("[write_tcp_state]id:%d cur state:%d, next state:%d\n", id, ta->connect_status[index].state, state);
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
            log_debug("[find_index]id:%d index:%d\n", id, *index);
            return TANK_SUCCESS;
        }
    }
    log_error("[find_index]can not find id:%d\n", id);
    return TANK_FAIL;
}

tank_status_t tank_app_creat(ta_info_t *ta, tank_id_t id, ta_protocol_t protocol, ta_type_t type)
{
    log_info("============ app init start ===========\n");
    memset(ta, 0, sizeof(ta_info_t));
    ta->id_cur_index = 0;
    ta->id = id;
    ta->protocol = protocol;
    ta->type = type;
    ta->send_package_cur_index = 0;
    snprintf(ta->name, 32, "APP%d", ta->id);
    app_allocate_heap(ta);
    app_allocate_msgq(ta);

    log_info("============ app init OK ===========\n");
    return TANK_SUCCESS;
}

tank_status_t ta_send_package(ta_info_t *ta, tank_id_t dst_id, void *package, uint32_t size, uint32_t tiemout)
{
    ta->send_package_status[ta->send_package_cur_index].src_id = ta->id;
    ta->send_package_status[ta->send_package_cur_index].dst_id = dst_id;
    ta->send_package_status[ta->send_package_cur_index].package = package;
    ta->send_package_status[ta->send_package_cur_index].size = size;
    ta->send_package_status[ta->send_package_cur_index].state = SEND_WAIT_REQUEST;

    log_info("[%s]ta_send_package start, src_id:%d, dst_id:%d, size:%d\n",
        ta->name, ta->id, dst_id, size);
    ta->send_package_cur_index += 1;
    return TANK_SUCCESS;
}
tank_status_t send_package_request(ta_info_t *ta, uint16_t index)
{
    tank_status_t send_status = 0;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_SEND_PACKAGE_REQUEST;
    info.send_package_request.src_id = ta->send_package_status[index].src_id;
    info.send_package_request.dst_id = ta->send_package_status[index].dst_id;
    info.send_package_request.size = ta->send_package_status[index].size;
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    ta->send_package_status[index].state = SEND_WAIT_ALLOCATE;
    if(send_status == TANK_FAIL){
        log_error("[%s]send_package_request, src_id:%d, dst_id:%d, size:%d\n",
                ta->name, info.send_package_request.src_id, info.send_package_request.dst_id, info.send_package_request.size
                );
    }
    log_info("[%s]send_package_request, src_id:%d, dst_id:%d, size:%d\n",
            ta->name, info.send_package_request.src_id, info.send_package_request.dst_id, info.send_package_request.size
            );
    return TANK_SUCCESS;
}
tank_status_t get_package_allocate(ta_info_t *ta, uint16_t index, app_request_info_t *info)
{
    addr_t addr = 0;

    if(info->send_package_allocate.dst_id == ta->id){
        addr = info->send_package_allocate.addr_shift + g_shm_base;
        ta->send_package_status[index].package_id = info->send_package_allocate.package_id;
    }else{
        log_info("[%s]recv id is error, get_package_allocate dst id %d\n", ta->name, info->send_package_allocate.dst_id);
        return TANK_FAIL;
    }
    log_info("[%s]get_package_allocate, package_id:%d shift:%d\n", ta->name, ta->send_package_status[index].package_id, info->send_package_allocate.addr_shift);
    memcpy((void*)addr, ta->send_package_status[index].package, ta->send_package_status[index].size);
    log_info("[%s]write data into, package_id:%d shift:%d\n", ta->name, ta->send_package_status[index].package_id, info->send_package_allocate.addr_shift);
    ta->send_package_status[index].state = SEND_FINISHE;
    return TANK_SUCCESS;
}
tank_status_t send_package_finished(ta_info_t *ta, uint16_t index)
{
    tank_status_t send_status = 0;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_SEND_PACKAGE_FINISHED;
    info.send_package_finshed.package_id = ta->send_package_status[index].package_id;
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s]send_package_finished, package_id:%d\n",
                ta->name, info.send_package_finshed.package_id
                );
    }
    log_info("[%s]send_package_finished, package_id:%d\n",
            ta->name, info.send_package_finshed.package_id
            );
    ta->send_package_status[index].state = SEND_IDLE;
    return TANK_SUCCESS;
}
tank_status_t get_package_finished(ta_info_t *ta, uint32_t package_id)
{
    tank_status_t send_status = 0;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_GET_PACKAGE_FINISHED;
    info.get_package_finished.package_id = package_id;
    send_status = tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    if(send_status == TANK_FAIL){
        log_error("[%s]get_package_finished, package_id:%d\n",
                ta->name, info.send_package_finshed.package_id
                );
    }
    log_info("[%s]get_package_finished, package_id:%d\n",
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
    info.type = APP_GET_PACKAGE_FINISHED;
    info.get_package_finished.package_id = package_id;
    tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s]get_package_finished, package_id:%d\n", package_id);
    return TANK_SUCCESS;
}
// tank_status_t tank_app_send_package_wait(ta_info_t *ta, tank_id_t dst_id, void* buf, uint32_t size, uint16_t timeout)
// {
//     app_request_info_t info;
//     if(ta->id == dst_id){
//         log_error("dst ID is self ID, check it!\n");
//         return TANK_FAIL;
//     }
//     memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
//     info.type = APP_SEND_PACKAGE;
//     info.package.src_id = ta->id;
//     info.package.dst_id = INNER_SERVICE_ID;
//     info.package.
//     tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
//     log_info("[%s]send a msg, src_id:%d, dst_id:%d, flag:%d\n",
//             ta->name, ta->id, dst_id, flag
//             );
//     return TANK_SUCCESS;
// }
tank_status_t tank_app_send_msg(ta_info_t *ta, tank_id_t dst_id, tcp_header_flag_t flag)
{
    app_request_info_t info;
    if(ta->id == dst_id){
        return TANK_FAIL;
    }
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = APP_SEND_MSG;
    info.msg.src_id = ta->id;
    info.msg.dst_id = dst_id;
    info.msg.flag = flag;
    tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s]send a msg, src_id:%d, dst_id:%d, flag:%d\n",
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
    if(info.type == APP_SEND_MSG){
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

    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);
    if(info.type == APP_SEND_MSG){
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
    tank_msgq_creat(ta->receiver, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    log_info("[%s]recv msgq addr:%p\n", ta->name, ta->receiver);

    mm_size = sizeof(tank_msgq_t) + 10*sizeof(app_get_package_push_t);
    ta->recv_package = (tank_msgq_t*)tank_mm_malloc(&ta->mm_handler, mm_size);
    tank_msgq_creat(ta->recv_package, sizeof(app_get_package_push_t), 10);
    log_info("[%s]recv_package msgq addr:%p\n", ta->name, ta->recv_package);

    // ta->sender = (tank_msgq_t*)tank_mm_malloc(&ta->mm_handler, mm_size);
    // tank_msgq_creat(ta->sender, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    ta->sender = g_service_msgq;
    log_info("[%s]send msgq addr:%p\n", ta->name, ta->sender);

    app_request_info_t app_info;
    memset(&app_info, 0, TANK_MSGQ_NORMAL_SIZE);

    app_info.type = APP_PUSH_MSGQ_ADDR;
    app_info.msgq.id = ta->id;
    app_info.msgq.recv_shift = (uint32_t)ta->receiver - g_shm_base;
    app_info.msgq.send_shift = (uint32_t)ta->sender - g_shm_base;

    tank_msgq_send(ta->sender, &app_info, TANK_MSGQ_NORMAL_SIZE);
    log_info("id:%d, type:%d, recv_shift:%d, send_shift:%d\n", app_info.msgq.id, app_info.type, app_info.msgq.recv_shift, app_info.msgq.send_shift);
    log_info("[%s]remain:%d\n", ta->mm_handler.name, ta->mm_handler.heap.xFreeBytesRemaining);
    return TANK_SUCCESS;
}

static tank_status_t app_allocate_heap(ta_info_t *ta)
{
    if(g_shm_baseet_flag_s == 0){
        get_shm_base_addr();
        g_shm_baseet_flag_s = 1;
    }
    get_service_msgq_addr(&g_service_msgq);

    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = MM_ALLOCATE;
    info.heap.id = ta->id;
    info.heap.size = APP_HEAP_SIZE;
    log_info("id:%d, len:%d\n", info.heap.id, info.heap.size);

    tank_msgq_send(g_service_msgq, &info, TANK_MSGQ_NORMAL_SIZE);

    log_info("start send malloc request\n");
    log_info("wait for backstage allocate finished\n");
    app_heap_get_t *app_get = (app_heap_get_t *)(SEM_ADDR);
    my_sem_wait(&app_get->sem);

    char name[48] = {0};
    snprintf(name, 48, "%s_mm", ta->name);
    tank_mm_register(&ta->mm_handler, g_shm_base+app_get->shift , APP_HEAP_SIZE, name);


    log_info("backstage allocate OK\n");
    log_info("get allocate info\n");
    log_info("addr_base:%p, shift:%d\n", (void*)g_shm_base, app_get->shift);
    return TANK_SUCCESS;
}



// static tank_status_t slice_package(uint16_t size, )