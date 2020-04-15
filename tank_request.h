#ifndef __TANK_REQUEST_H__
#define __TANK_REQUEST_H__

#include "tank_pub.h"
#include "tank_app_pub.h"


typedef uint16_t tank_package_slice_t;
typedef uint16_t tank_package_size_t;
typedef uint32_t tank_package_addr_t;


typedef struct{
    tank_id_t id;
    uint32_t recv_shift;
    uint32_t send_shift;
}app_msgq_info_t;

typedef struct{
    tank_id_t src_id;
    tank_id_t dst_id;
    tcp_header_flag_t flag;
}app_send_msg_t;

typedef struct{
    tank_id_t src_id;
    tank_id_t dst_id;
    tank_id_t package_id;
    tank_package_size_t size;
    tank_package_addr_t addr_shift;
}app_package_t;

typedef struct{
    tank_id_t src_id;
    tank_id_t dst_id;
    tank_package_size_t size;
}app_send_package_request_t;


typedef struct{
    tank_id_t dst_id;
    tank_id_t package_id;
    addr_shift_t addr_shift;
}app_get_package_allocate_t;

typedef struct{
    tank_id_t package_id;
}app_send_package_finished_t;

typedef app_package_t app_get_package_push_t;

typedef struct{
    tank_id_t package_id;
}app_get_package_finsihed_t;

typedef enum{
    MM_ALLOCATE = 0,
    MM_FREE,
    APP_PUSH_MSGQ_ADDR,
    APP_SEND_MSG,
    APP_SEND_PACKAGE_REQUEST,
    APP_GET_PACKAGE_ALLOCATE,
    APP_SEND_PACKAGE_FINISHED,

    APP_GET_PACKAGE_PUSH,
    APP_GET_PACKAGE_FINISHED,
    PUSH_ERROR
}app_request_type_t;


typedef struct{
    tank_id_t id;
    uint32_t size;
}app_heap_request_t;

// socket获得的ID和偏移量
typedef struct{
    my_sem_t sem;
    uint32_t shift;
}app_heap_get_t;


#define APP_MSG_SIZE (sizeof(app_request_type_t)+sizeof(app_send_msg_t))
#define TANK_MSG_NORMAL_SIZE (20)


typedef struct{
    app_request_type_t type;
    union{
        app_heap_request_t heap;
        app_msgq_info_t msgq;
        app_send_msg_t msg;
        app_send_package_request_t send_package_request;
        app_get_package_allocate_t send_package_allocate;
        app_send_package_finished_t send_package_finshed;

        app_get_package_push_t get_package_push;
        app_get_package_finsihed_t get_package_finished;
    };
}app_request_info_t;







#endif