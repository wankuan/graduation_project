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

tank_status_t tank_app_creat(ta_info_t *ta, const char * name, ta_protocol_t protocol, ta_type_t type)
{
    ta->protocol = protocol;
    ta->type = type;
    strncpy(ta->name, name, 32);
    app_allocate_heap(ta);
    app_allocate_msgq(ta);
    return TANK_SUCCESS;
}
static tank_status_t app_allocate_msgq(ta_info_t *ta)
{
    uint16_t mm_size = sizeof(tank_msgq_t)+TANK_MSGQ_NORMAL_LEN*TANK_MSGQ_NORMAL_SIZE;

    ta->receiver = (tank_msgq_t*)tank_mm_malloc(&ta->mm_handler, mm_size);
    tank_msgq_creat(ta->receiver, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    printf("[%s] recv msgq addr:%p\n", ta->name, ta->receiver);


    ta->sender = (tank_msgq_t*)tank_mm_malloc(&ta->mm_handler, mm_size);
    tank_msgq_creat(ta->sender, TANK_MSGQ_NORMAL_SIZE, TANK_MSGQ_NORMAL_LEN);
    printf("[%s] send msgq addr:%p\n", ta->name, ta->sender);

    app_request_info_t app_info;
    app_info.type = PUSH_MSGQ_ADDR;
    app_info.msgq.id = ta->id;
    app_info.msgq.recv_shift = (uint32_t)ta->receiver - g_shm_base;
    app_info.msgq.send_shift = (uint32_t)ta->sender - g_shm_base;

    tank_msgq_send(g_service_msgq, &app_info, sizeof(app_request_info_t));
    printf("[app_info_push]id:%d, type:%d, recv_shift:%d, send_shift:%d\n", app_info.msgq.id, app_info.type, app_info.msgq.recv_shift, app_info.msgq.send_shift);
    printf("[%s]remain:%d\n", ta->mm_handler.name, ta->mm_handler.heap.xFreeBytesRemaining);
    return TANK_SUCCESS;
}

static tank_status_t app_allocate_heap(ta_info_t *ta)
{
    app_request_info_t app_info;

    if(g_shm_baseet_flag_s == 0){
        get_service_base_addr();
        g_shm_baseet_flag_s = 1;
    }
    get_service_msgq_addr(&g_service_msgq);

    app_info.type = MM_ALLOCATE;
    memset(&app_info.request, 0, sizeof(app_info.request));
    strncpy(app_info.request.name, ta->name, TA_NAME_SIZE_MAX);
    app_info.request.size = APP_HEAP_SIZE;
    printf("[APP]name:%s, len:%d\n", app_info.request.name, app_info.request.size);

    tank_msgq_send(g_service_msgq, &app_info, sizeof(app_request_info_t));

    printf("[APP]start send malloc request\n");
    printf("[APP]wait for backstage allocate finished\n");
    app_heap_get_t *app_get = (app_heap_get_t *)(SEM_ADDR);
    my_sem_wait(&app_get->sem);

    char name[16] = {0};
    snprintf(name, 16, "%s_mm", ta->name);
    tank_mm_register(&ta->mm_handler, g_shm_base+app_get->shift , APP_HEAP_SIZE, name);
    ta->id = app_get->id;

    printf("[APP]backstage allocate OK\n");
    printf("[APP]get allocate info\n");
    printf("[APP]id:%d, addr_base:%p, shift:%d\n", app_get->id, (void*)g_shm_base, app_get->shift);
    return TANK_SUCCESS;
}