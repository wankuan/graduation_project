#include "tank_msgq.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */


tank_status_t tank_msgq_creat(tank_msgq_t *handler, msgq_size_t size, msgq_len_t len)
{
    handler->size = size;
    handler->len = len;
    handler->buf = NULL;
    handler->head = 0;
    handler->tail = 0;
    handler->buf = (void*)handler + sizeof(tank_msgq_t);
    my_sem_creat(&handler->sem_rw, 1);
    my_sem_creat(&handler->sem_cur_len, 0);
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
    my_sem_wait(&handler->sem_rw);
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    my_sem_wait(&handler->sem_cur_len);
    printf("\ntank_msgq_recv  tail:%d cur_len:%d\n\n", handler->tail, val);
    memcpy(msg, (&handler->buf + handler->tail*handler->size), len);

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
    printf("[MSGQ]recv,tail:%d,cur_len:%d\n", handler->tail, val);
    memcpy(msg, (&handler->buf + handler->tail*handler->size), len);

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
        printf("[ERROR]msgq is full\n");
        return TANK_FAIL;
    }
    my_sem_wait(&handler->sem_rw);
    if(!tank_msgq_is_empty(handler)){
        handler->head += 1;
        handler->head %= handler->len;
    }
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    printf("[MSGQ]send,head:%d,cur_len:%d\n", handler->head, val);
    memcpy((&handler->buf + handler->head*handler->size), msg, len);
    my_sem_post(&handler->sem_cur_len);
    my_sem_post(&handler->sem_rw);
    return TANK_SUCCESS;
}

uint8_t tank_msgq_is_full(tank_msgq_t *handler)
{
    int val = 0;
    my_sem_get_val(&handler->sem_cur_len, &val);
    if(val == handler->len){
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