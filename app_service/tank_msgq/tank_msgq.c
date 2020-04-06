#include "tank_msgq.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */


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

tank_status_t tank_msgq_creat(tank_msgq_t *handler, msgq_size_t size, msgq_len_t len)
{
    handler->size = size;
    handler->len = len;
    handler->cur_len = 0;
    handler->buf = NULL;
    handler->head = 0;
    handler->tail = 0;
    handler->buf = (void*)handler + sizeof(tank_msgq_t);
    my_sem_creat(&handler->sem, 1);
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
    return TANK_SUCCESS;
}


tank_status_t tank_msgq_recv(tank_msgq_t* handler, void *msg, msgq_len_t len)
{
    if(tank_msgq_is_empty(handler)){
        printf("[ERROR]msgq is empty\n");
        return TANK_FAIL;
    }
    my_sem_wait(&handler->sem);
    printf("\ntank_msgq_recv  tail:%d cur_len:%d\n", handler->tail, handler->cur_len);
    memcpy(msg, (&handler->buf + handler->tail*handler->size), len);
    handler->cur_len -= 1;
    if(!tank_msgq_is_empty(handler)){
        handler->tail += 1;
        handler->tail %= handler->len;
    }
    my_sem_post(&handler->sem);
    return TANK_SUCCESS;
}
tank_status_t tank_msgq_recv_wait(tank_msgq_t* handler, void *msg, msgq_len_t len,uint16_t timeout)
{
    return TANK_SUCCESS;
}
tank_status_t tank_msgq_send(tank_msgq_t* handler, void *msg, msgq_len_t len)
{
    if(tank_msgq_is_full(handler)){
        printf("[ERROR]msgq is full\n");
        return TANK_FAIL;
    }
    my_sem_wait(&handler->sem);
    if(!tank_msgq_is_empty(handler)){
        handler->head += 1;
        handler->head %= handler->len;
    }
    handler->cur_len += 1;
    printf("\ntank_msgq_send  head:%d cur_len:%d\n", handler->head, handler->cur_len);
    memcpy((&handler->buf + handler->head*handler->size), msg, len);
    my_sem_post(&handler->sem);
    return TANK_SUCCESS;
}

uint8_t tank_msgq_is_full(tank_msgq_t *handler)
{
    if(handler->cur_len == handler->len){
        return 1;
    }else{
        return 0;
    }
}
uint8_t tank_msgq_is_empty(tank_msgq_t *handler)
{
    if(handler->cur_len == 0){
        return 1;
    }else{
        return 0;
    }
}