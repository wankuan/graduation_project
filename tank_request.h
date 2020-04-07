#ifndef __TANK_REQUEST_H__
#define __TANK_REQUEST_H__

#include "tank_pub.h"


typedef enum{
    MM_ALLOCATE = 0,
    MM_FREE,
    PUSH_MSGQ_ADDR,
    PUSH_ERROR
}app_request_type_t;


typedef struct{
    char name[8];
    uint32_t size;
}app_heap_request_t;

// socket获得的ID和偏移量
typedef struct{
    my_sem_t sem;
    uint16_t id;
    uint32_t shift;
}app_heap_get_t;


typedef struct{
    app_request_type_t type;
    app_heap_request_t request;
}app_info_t;

typedef struct{
    uint16_t id;
    uint32_t shift;
}socket_heap_get_t;

typedef struct{
    uint16_t id;
    uint32_t shift;
    char name[8];
}app_allocate_info_t;



#endif