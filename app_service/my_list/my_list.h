#ifndef __MY_LIST_H__
#define __MY_LIST_H__
#include "tank_pub.h"
#include "tank_mm.h"

typedef struct list_node_{
    struct list_node_ *next;
    uint8_t       data[0];
}list_node_t;


typedef struct{
    list_node_t *head;
    tank_mm_t *mm;
    uint16_t data_size;
    uint16_t cur_len;
    uint16_t max_len;
}list_head_t;


tank_status_t list_creat(list_head_t *handler, tank_mm_t *mm, uint16_t data_size, uint16_t max_len);

tank_status_t list_destory(list_head_t *handler);

tank_status_t list_rewrite_node(list_head_t *handler, uint16_t index, void *nex_data);

tank_status_t list_add_node(list_head_t *handler, void *data);

list_node_t *list_get_last_node(list_head_t *handler);

tank_status_t list_get_node(list_head_t *handler, uint16_t index, void *data);

tank_status_t list_delete_node(list_head_t *handler, uint16_t index);

tank_status_t list_is_full(list_head_t *handler);

tank_status_t list_is_empty(list_head_t *handler);

uint16_t list_get_len(list_head_t *handler);

#endif