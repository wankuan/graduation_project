#ifndef __TANK_REQUEST_H__
#define __TANK_REQUEST_H__

#include "tank_pub.h"
#include "tank_app_pub.h"


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


typedef enum{
    MM_ALLOCATE = 0,
    MM_FREE,
    PUSH_MSGQ_ADDR,
    SEND_MSG,
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
    };
}app_request_info_t;







#endif