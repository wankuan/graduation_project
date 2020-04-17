#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "tank_pub.h"

typedef uint16_t ringbuffer_len_t;

typedef struct{
    ringbuffer_len_t len;
    ringbuffer_len_t cur_len;
    ringbuffer_len_t size;
    ringbuffer_len_t head;
    ringbuffer_len_t tail;
    void             *buf;
}ringbuffer_t;

tank_status_t ringbuffer_creat(ringbuffer_t *handler, ringbuffer_len_t size, ringbuffer_len_t len);

tank_status_t ringbuffer_delete(ringbuffer_t *handler);

tank_status_t ringbuffer_recv(ringbuffer_t *handler, void *msg, ringbuffer_len_t size);

tank_status_t ringbuffer_send(ringbuffer_t *handler, void *msg, ringbuffer_len_t size);

tank_status_t ringbuffer_is_full(ringbuffer_t *handler);
tank_status_t ringbuffer_is_empty(ringbuffer_t *handler);


#endif