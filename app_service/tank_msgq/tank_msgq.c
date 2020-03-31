#include "tank_msgq.h"

tank_msgq_map_t *msgq_map_s;

tank_status_t tank_msgq_constructor(void)
{
    msgq_map_s = (tank_msgq_map_t*)TANK_MSGQ_MAP_ADDR_START;
    return TANK_SUCCESS;
}
tank_msgq_t *tank_get_msgq_addr(tank_queue_id_t id)
{
    if(id > TANK_MAX_SIZE){
        return NULL;
    }
    return msgq_map_s->addr[id];
}

tank_status_t tank_msgq_creat(tank_msgq_t* handler, tank_msgq_len_t len)
{
    handler->len = len;
    handler->buf = NULL;
    handler->head = 0;
    handler->tail = 0;
    handler->buf = malloc(len*TANK_MSGQ_MAX_SIZE);
    if(handler->buf == NULL){
        tank_msgq_delete(handler);
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}

tank_status_t tank_msgq_delete(tank_msgq_t* handler)
{
    free(handler->buf);
    free(handler);
}


tank_status_t tank_msgq_recv(tank_msgq_t* handler, uint8_t *msg, uint16_t len);
tank_status_t tank_msgq_recv_wait(tank_msgq_t* handler, uint8_t *msg, uint16_t len,uint16_t timeout);
tank_status_t tank_msgq_send(tank_msgq_t* handler, uint8_t *msg, uint16_t len);