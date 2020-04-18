#ifndef __INNER_SWAP_H__
#define __INNER_SWAP_H__

#include "tank_pub.h"
#include "tank_request.h"
#include "tank_map.h"


typedef struct{
    tank_id_t id;
    void *heap_addr;
    void *msgq_recv_addr;
    void *msgq_send_addr;
}app_info_t;


tank_status_t inner_service_init(void);

tank_status_t inner_service_deinit(void);


#endif