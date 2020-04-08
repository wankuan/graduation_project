#ifndef __INNER_SWAP_H__
#define __INNER_SWAP_H__

#include "tank_pub.h"
#include "tank_request.h"
#include "tank_map.h"


typedef struct{
    uint16_t id;
    void *heap_addr;
    void *msgq_recv_addr;
    void *msgq_send_addr;
    char name[16];
}app_info_t;



#endif