#ifndef __TANK_REQUEST_H__
#define __TANK_REQUEST_H__

#include "tank_pub.h"
#include "tank_app_pub.h"


typedef uint16_t tank_package_slice_t;
typedef uint16_t tank_package_size_t;
typedef uint32_t tank_package_addr_t;

typedef enum{
    NO_SYSTEM_ID,
    NO_PHY_LINK,
    NO_ACK,
    SEND_TIME_OUT,
    RECV_TIME_OUT
}tank_msg_error_t;



typedef struct{
    tank_id_t id;
    uint32_t recv_shift;
    uint32_t send_shift;
}app_msgq_info_t;

typedef struct{
    tank_id_t src_id;
    tank_id_t dst_id;
    tcp_header_flag_t flag;
}app_send_tcp_state_t;


typedef enum{
    REQUEST_MM,
    SENDING,
    RESTRANSIMTING,
    FINISHED
}package_state_t;

typedef struct{
    tank_id_t src_id;
    tank_id_t dst_id;
    tank_id_t package_id;
    tank_package_size_t size;
    tank_package_addr_t addr_shift;
    package_state_t state;
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
}app_recv_package_allocate_t;

typedef struct{
    tank_id_t package_id;
}app_send_package_finished_t;

typedef app_package_t app_recv_package_push_t;

typedef struct{
    tank_id_t package_id;
}app_recv_package_ack_t;

typedef struct{
    tank_id_t package_id;
}app_recv_package_finished_t;

typedef enum{
    MM_ALLOCATE                 = 2,
    MM_FREE                     = 3,
    HEART_BEAT                  = 4,
    APP_PUSH_MSGQ_ADDR,
    APP_TCP_MSG,
    APP_SEND_PACKAGE_REQUEST,
    APP_GET_PACKAGE_ALLOCATE,
    APP_SEND_PACKAGE_FINISHED,

    APP_recv_package_push,
    APP_GET_PACKAGE_ACK,
    APP_READ_PACKAGE_FINISHED,
    PUSH_ERROR
}app_request_type_t;


typedef struct{
    tank_id_t id;
    uint32_t size;
}app_heap_request_t;


// 后台向APP推送分配结果
typedef struct{
    my_sem_t sem;
    uint32_t shift;
}inner_service_push_heap_t;


#define APP_MSG_SIZE (sizeof(app_request_type_t)+sizeof(app_send_tcp_state_t))
#define TANK_MSG_NORMAL_SIZE (20)


typedef struct{
    uint32_t src_id;
    uint32_t value;
}app_heart_beat_t;


typedef struct{
    app_request_type_t type;
    union{
        app_heart_beat_t heart_beat;
        app_heap_request_t heap;
        app_msgq_info_t msgq;

        app_send_tcp_state_t tcp_state;
        app_send_package_request_t send_package_request;
        app_recv_package_allocate_t recv_package_allocate;
        app_send_package_finished_t send_package_finished;

        app_recv_package_push_t recv_package_push;
        app_recv_package_ack_t recv_package_ack;
        app_recv_package_finished_t recv_package_finished;
    };
}app_request_info_t;


typedef struct{
    tank_id_t                   id;
    app_request_info_t request_info;
}external_msg_info_t;




#endif