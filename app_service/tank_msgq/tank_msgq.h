#ifndef __TANK_MSGQ_H__
#define __TANK_MSGQ_H__

#include "tank_pub.h"



#define TANK_MSGQ_MAX_SIZE 20
#define TANK_MSGQ_MAP_ADDR ((uint32_t*)0x12345678)

#define TANK_MSGQ_MAP_ADDR_START ((uint32_t*)(0x12345678 - 2))

typedef uint16_t tank_msgq_len_t;
typedef uint8_t (*msgq_t)[TANK_MSGQ_MAX_SIZE];

typedef struct{
    tank_id_t len;
    tank_id_t head;
    tank_id_t tail;
    msgq_t buf;
}tank_msgq_t;

typedef struct{
    tank_id_t len;
    tank_msgq_t *addr[0];
}tank_msgq_map_t;

extern tank_msgq_map_t *msgq_map_s;





tank_msgq_t *tank_get_msgq_addr(tank_queue_id_t id);

tank_status_t tank_msgq_constructor(void);

tank_status_t tank_msgq_creat(tank_msgq_t* handler, tank_msgq_len_t len);
tank_status_t tank_msgq_delete(tank_msgq_t* handler);


tank_status_t tank_msgq_recv(tank_msgq_t* handler, uint8_t *msg, uint16_t len);
tank_status_t tank_msgq_recv_wait(tank_msgq_t* handler, uint8_t *msg, uint16_t len,uint16_t timeout);
tank_status_t tank_msgq_send(tank_msgq_t* handler, uint8_t *msg, uint16_t len);
#endif