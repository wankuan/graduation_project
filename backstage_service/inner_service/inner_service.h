#ifndef __INNER_SWAP_H__
#define __INNER_SWAP_H__

#include "tank_pub.h"
#include "tank_request.h"
#include "tank_map.h"



tank_status_t ts_malloc_heap(my_sem_t sem, socket_heap_request_t *request, socket_heap_get_t *get);


#endif