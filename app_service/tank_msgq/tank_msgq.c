#include "tank_msgq.h"
<<<<<<< HEAD

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
=======
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include "tank_log_api.h"
#define FILE_NAME "tank_msgq"
tank_status_t tank_msgq_creat(tank_msgq_t *handler, msgq_size_t size, msgq_len_t len)
{
    handler->size = size;
    handler->len = len;
    handler->buf_shift = 0;
    handler->head = 0;
    handler->tail = 0;
    handler->buf_shift = sizeof(tank_msgq_t);
    my_sem_creat(&handler->sem_rw, 1);
    my_sem_creat(&handler->sem_cur_len, 0);
    if(handler->buf_shift == 0){
>>>>>>> dev
        tank_msgq_delete(handler);
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}

tank_status_t tank_msgq_delete(tank_msgq_t* handler)
{
<<<<<<< HEAD
    free(handler->buf);
    free(handler);
}


tank_status_t tank_msgq_recv(tank_msgq_t* handler, uint8_t *msg, uint16_t len);
tank_status_t tank_msgq_recv_wait(tank_msgq_t* handler, uint8_t *msg, uint16_t len,uint16_t timeout);
tank_status_t tank_msgq_send(tank_msgq_t* handler, uint8_t *msg, uint16_t len);
=======
    // free(handler->buf);
    free(handler);
    return TANK_SUCCESS;
}


tank_status_t tank_msgq_recv(tank_msgq_t* handler, void *msg, msgq_len_t len)
{
    if(tank_msgq_is_empty(handler)){
        log_error("msgq is empty\n");
        return TANK_FAIL;
    }
    my_sem_wait(&handler->sem_rw);
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    my_sem_wait(&handler->sem_cur_len);
    log_info("\ntank_msgq_recv  tail:%d cur_len:%d\n\n", handler->tail, val);

    void *buf = (void*)handler + handler->buf_shift + handler->tail*handler->size;
    memcpy(msg, buf, len);
    memset(buf, 0, handler->size);

    if(!tank_msgq_is_empty(handler)){
        handler->tail += 1;
        handler->tail %= handler->len;
    }
    my_sem_post(&handler->sem_rw);
    return TANK_SUCCESS;
}

tank_status_t tank_msgq_recv_wait(tank_msgq_t* handler, void *msg, msgq_len_t len)
{
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    my_sem_wait(&handler->sem_cur_len);
    my_sem_wait(&handler->sem_rw);
    log_debug("recv,tail:%d,cur_len:%d\n", handler->tail, val);

    void *buf = (void*)handler + handler->buf_shift + handler->tail*handler->size;
    memcpy(msg, buf, len);
    memset(buf, 0, handler->size);

    if(!tank_msgq_is_empty(handler)){
        handler->tail += 1;
        handler->tail %= handler->len;
    }
    my_sem_post(&handler->sem_rw);
    return TANK_SUCCESS;
}
tank_status_t tank_msgq_recv_timeout(tank_msgq_t* handler, void *msg, msgq_len_t len,uint16_t timeout)
{
    return TANK_SUCCESS;
}
tank_status_t tank_msgq_send(tank_msgq_t* handler, void *msg, msgq_len_t len)
{
    if(tank_msgq_is_full(handler)){
        log_error("msgq is full\n");
        return TANK_FAIL;
    }
    my_sem_wait(&handler->sem_rw);
    if(!tank_msgq_is_empty(handler)){
        handler->head += 1;
        handler->head %= handler->len;
    }
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    log_debug("send,head:%d,cur_len:%d\n", handler->head, val);

    void *buf = (void*)handler + handler->buf_shift + handler->head*handler->size;

    memset(buf, 0, handler->size);

    memcpy(buf, msg, len);
    my_sem_post(&handler->sem_rw);
    my_sem_post(&handler->sem_cur_len);
    return TANK_SUCCESS;
}

uint8_t tank_msgq_is_full(tank_msgq_t *handler)
{
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    if(val >= handler->len){
        return 1;
    }else{
        return 0;
    }
}
uint8_t tank_msgq_is_empty(tank_msgq_t *handler)
{
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    if(val == 0){
        return 1;
    }else{
        return 0;
    }
}
>>>>>>> dev
