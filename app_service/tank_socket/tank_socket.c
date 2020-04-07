#include "tank_socket.h"
#include "tank_socket_pub.h"
#include "tank_msgq.h"
#include "tank_map.h"
#include "tank_request.h"

static tank_status_t sock_allocate_heap(ts_info_t *ts);

uint8_t shm_base_set_flag_s = 0;

tank_status_t tank_socket_creat(ts_info_t *ts, const char * name, ts_protocol_t protocol, ts_type_t type)
{
    ts->mm_handler.heap.total_size = 512;
    strncpy(ts->name, name, 16);
    sock_allocate_heap(ts);
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
    app_info.request.size = ts->mm_handler.heap.total_size;
    printf("[APP]name:%s, len:%d\n", app_info.request.name, app_info.request.size);

    tank_msgq_send(service_msgq, &app_info, sizeof(app_info_t));

    printf("[APP]start send malloc request\n");
    printf("[APP]wait for backstage allocate finished\n");
    app_heap_get_t *app_get = (app_heap_get_t *)(SEM_ADDR);
    my_sem_wait(&app_get->sem);

    printf("[APP]backstage allocate OK\n");
    printf("[APP]get allocate info\n");
    printf("[APP]id:%d, addr_base:%p, shift:%d\n", app_get->id, (void*)shm_base_s, app_get->shift);
    return TANK_SUCCESS;
}