#include "ring_buffer.h"

#include "tank_log_api.h"
#define FILE_NAME "tank_app"

tank_status_t ringbuffer_creat(ringbuffer_t *handler, ringbuffer_len_t size, ringbuffer_len_t len)
{
    handler->size = size;
    handler->len = len;
    handler->cur_len = 0;
    handler->head = 0;
    handler->tail = 0;
    if(handler->buf == NULL){
        log_error("ringbuffer_t not allocate memory\n");
    }
    return TANK_SUCCESS;
}

tank_status_t ringbuffer_get_len(ringbuffer_t *handler, ringbuffer_len_t* len)
{
    *len = handler->cur_len;
    return TANK_SUCCESS;
}


tank_status_t ringbuffer_get_data(ringbuffer_t *handler, ringbuffer_len_t index, void *data, ringbuffer_len_t size)
{
    ringbuffer_len_t cur_index = 0;
    void* data_addr = NULL;
    if(ringbuffer_is_empty(handler)){
        log_error("ring buffer is empty\n");
        return TANK_FAIL;
    }
    cur_index = handler->tail + index;
    cur_index %= handler->len;
    data_addr = handler->buf + cur_index*handler->size;
    memcpy(data, data_addr, size);
    memset(data_addr, 0, handler->size);
    if(!ringbuffer_is_empty(handler)){
        handler->tail += 1;
        handler->tail %= handler->len;
    }
    return TANK_SUCCESS;
}
tank_status_t ringbuffer_delete(ringbuffer_t *handler)
{

}

tank_status_t ringbuffer_recv(ringbuffer_t *handler, void *msg, ringbuffer_len_t size)
{

}

tank_status_t ringbuffer_send(ringbuffer_t *handler, void *msg, ringbuffer_len_t size)
{

}

tank_status_t ringbuffer_is_full(ringbuffer_t *handler)
{
    if(handler->cur_len == handler->len){
        return TANK_SUCCESS;
    }
    return TANK_FAIL;
}
tank_status_t ringbuffer_is_empty(ringbuffer_t *handler)
{
    if(handler->cur_len == 0){
        return TANK_SUCCESS;
    }
    return TANK_FAIL;
}
