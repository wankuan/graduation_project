#ifndef __TANK_MSGQ_H__
#define __TANK_MSGQ_H__

#include "tank_pub.h"
#include "my_sem.h"
#include "tank_mm.h"
#ifdef __LINUX__
#define tank_msgq_map_name "/tank_msgq_map"
#endif

#define TANK_TANK_MSGQ_NUM 255
#define TANK_MSGQ_BUFFER_SIZE 20
#define TANK_MSGQ_MAP_ADDR ((uint32_t*)0x12345678)

#define TANK_MSGQ_MAP_ADDR_START ((uint32_t*)(0x12345678 - 2))

typedef uint16_t tank_msgq_len_t;
typedef uint8_t (*msgq_t)[TANK_MSGQ_BUFFER_SIZE];

typedef struct msgq_data_{
    void *data;
    uint16_t size;
    struct msgq_data_ *next;
}msgq_data_t;

typedef uint16_t msgq_len_t;
typedef uint16_t msgq_size_t;
typedef struct{
    my_sem_t sem_rw;
    my_sem_t sem_cur_len;
    msgq_len_t len;
    msgq_size_t size;
    msgq_len_t head;
    msgq_len_t tail;
    void  *buf;
}tank_msgq_t;

typedef struct{
    uint32_t len;
    tank_msgq_t *addr[0];
}tank_msgq_map_t;




tank_msgq_t *tank_get_msgq_addr(tank_queue_id_t id);

tank_status_t tank_msgq_constructor(void);

tank_status_t tank_msgq_creat(tank_msgq_t *handler, msgq_size_t size, msgq_len_t len);

tank_status_t tank_msgq_delete(tank_msgq_t *handler);


tank_status_t tank_msgq_recv(tank_msgq_t *handler, void *msg, msgq_len_t len);
tank_status_t tank_msgq_recv_wait(tank_msgq_t* handler, void *msg, msgq_len_t len);
tank_status_t tank_msgq_recv_timeout(tank_msgq_t* handler, void *msg, msgq_len_t len,uint16_t timeout);
tank_status_t tank_msgq_send(tank_msgq_t *handler, void *msg, msgq_len_t len);

uint8_t tank_msgq_is_full(tank_msgq_t *handler);
uint8_t tank_msgq_is_empty(tank_msgq_t *handler);

#endif