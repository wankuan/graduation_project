#include "tank_socket.h"
#include "tank_socket_pub.h"
#include "tank_msgq.h"
#include "tank_map.h"
#include "tank_request.h"

#define APP_HEAP_SIZE 1024
static tank_status_t sock_allocate_heap(ts_info_t *ts);
static tank_status_t allocate_msgq(ts_info_t *ts);

uint8_t shm_base_set_flag_s = 0;

tank_status_t tank_socket_creat(ts_info_t *ts, const char * name, ts_protocol_t protocol, ts_type_t type)
{
    strncpy(ts->name, name, 16);
    sock_allocate_heap(ts);
    allocate_msgq(ts);
    return TANK_SUCCESS;
}
static tank_status_t allocate_msgq(ts_info_t *ts)
{
    uint16_t mm_size = sizeof(tank_msgq_t)+TANK_MSGQ_NORMAL_LEN*TANK_MSGQ_NORMAL_SIZE;

    ts->receiver = (tank_msgq_t*)tank_mm_malloc(&ts->mm_handler, mm_size);
    tank_msgq_creat(ts->receiver, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    printf("[%s] recv msgq addr:%p\n", ts->name, ts->receiver);

    ts->sender = (tank_msgq_t*)tank_mm_malloc(&ts->mm_handler, mm_size);
    tank_msgq_creat(ts->sender, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    printf("[%s] send msgq addr:%p\n", ts->name, ts->sender);

    printf("[%s]remain:%d\n", ts->mm_handler.name, ts->mm_handler.heap.xFreeBytesRemaining);
    return TANK_SUCCESS;
}

static tank_status_t sock_allocate_heap(ts_info_t *ts)
{
    app_info_t app_info;
    tank_msgq_t *service_msgq = NULL;

    if(shm_base_set_flag_s == 0){
        get_service_base_addr();
        shm_base_set_flag_s = 1;
    }
    get_service_msgq_addr(&service_msgq);

    app_info.type = MM_ALLOCATE;
    memset(&app_info.request, 0, sizeof(app_info.request));
    strncpy(app_info.request.name, ts->name, TS_NAME_SIZE_MAX);
    app_info.request.size = APP_HEAP_SIZE;
    printf("[APP]name:%s, len:%d\n", app_info.request.name, app_info.request.size);

    tank_msgq_send(service_msgq, &app_info, sizeof(app_info_t));

    printf("[APP]start send malloc request\n");
    printf("[APP]wait for backstage allocate finished\n");
    app_heap_get_t *app_get = (app_heap_get_t *)(SEM_ADDR);
    my_sem_wait(&app_get->sem);
    char name[16] = {0};
    snprintf(name, 16, "%s_mm", ts->name);
    tank_mm_register(&ts->mm_handler, shm_base_s+app_get->shift , APP_HEAP_SIZE, name);
    printf("[APP]backstage allocate OK\n");
    printf("[APP]get allocate info\n");
    printf("[APP]id:%d, addr_base:%p, shift:%d\n", app_get->id, (void*)shm_base_s, app_get->shift);
    return TANK_SUCCESS;
}