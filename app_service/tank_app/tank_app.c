#include "tank_app.h"
#include "tank_app_pub.h"
#include "tank_msgq.h"
#include "tank_map.h"
#include "tank_request.h"

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
    ta->index_lut[ta->cur_index].id = id;
    ta->index_lut[ta->cur_index].index = ta->cur_index;
    ta->cur_index += 1;
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
    for(int i=0; i<ta->cur_index; i++){
        if(ta->index_lut[i].id == id){
            return TANK_SUCCESS;
        }
    }
    return TANK_FAIL;
}
static tank_status_t find_tcp_state_index(ta_info_t *ta, tank_id_t id, tank_id_t *index)
{
    for(int i=0; i<ta->cur_index; i++){
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
    ta->cur_index = 0;
    ta->id = id;
    ta->protocol = protocol;
    ta->type = type;
    snprintf(ta->name, 32, "APP%d", ta->id);
    app_allocate_heap(ta);
    app_allocate_msgq(ta);
    log_info("============ app init OK ===========\n");
    return TANK_SUCCESS;
}
tank_status_t tank_app_send(ta_info_t *ta, tank_id_t dst_id, tcp_header_flag_t flag)
{
    app_request_info_t info;
    if(ta->id == dst_id){
        return TANK_FAIL;
    }
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    info.type = SEND_MSG;
    info.msg.src_id = ta->id;
    info.msg.dst_id = dst_id;
    info.msg.flag = flag;
    tank_msgq_send(ta->sender, &info, TANK_MSGQ_NORMAL_SIZE);
    log_info("[%s]send a msg, src_id:%d, dst_id:%d, flag:%d\n",
            ta->name, ta->id, dst_id, flag
            );
    return TANK_SUCCESS;
}

tank_status_t tank_app_recv(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag)
{
    app_request_info_t info;
    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    if(tank_msgq_recv(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE) == TANK_FAIL){
        return TANK_FAIL;
    }
    if(info.type == SEND_MSG){
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

tank_status_t tank_app_recv_wait(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag)
{

    memset(&info, 0, TANK_MSGQ_NORMAL_SIZE);
    tank_msgq_recv_wait(ta->receiver, &info, TANK_MSGQ_NORMAL_SIZE);
    if(info.type == SEND_MSG){
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
    log_info("[%s] recv msgq addr:%p\n", ta->name, ta->receiver);

    // ta->sender = (tank_msgq_t*)tank_mm_malloc(&ta->mm_handler, mm_size);
    // tank_msgq_creat(ta->sender, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    ta->sender = g_service_msgq;
    log_info("[%s] send msgq addr:%p\n", ta->name, ta->sender);

    app_request_info_t app_info;
    memset(&app_info, 0, TANK_MSGQ_NORMAL_SIZE);

    app_info.type = PUSH_MSGQ_ADDR;
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