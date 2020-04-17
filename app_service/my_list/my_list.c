#include "my_list.h"

#include "tank_log_api.h"
#define FILE_NAME "my_list"


tank_status_t list_creat(list_head_t *handler, tank_mm_t *mm, uint16_t data_size, uint16_t max_len)
{
    handler->data_size = data_size;
    handler->max_len = max_len;
    handler->head = NULL;
    handler->cur_len = 0;
    handler->mm = mm;
    return TANK_SUCCESS;
}

tank_status_t list_destory(list_head_t *handler)
{

}

tank_status_t list_add_node(list_head_t *handler, void *data)
{
    if(list_is_full(handler)){
        log_error("list reach max len\n");
        return TANK_FAIL;
    }
    list_node_t *node = tank_mm_calloc(handler->mm, handler->data_size + sizeof(void*));
    if(node == NULL){
        log_error("node allocate memory fail, heap full\n");
    }
    node->next = NULL;
    memcpy(node->data, data, handler->data_size);
    list_node_t *last_node = list_get_last_node(handler);
    if(last_node == NULL){
        handler->head = node;
    }else{
        last_node->next = node;
    }
    handler->cur_len += 1;
    return TANK_SUCCESS;
}
tank_status_t list_rewrite_node(list_head_t *handler, uint16_t index, void *nex_data)
{
    if(list_is_empty(handler)){
        log_error("list is empty, can not get node\n");
        return TANK_FAIL;
    }
    if(list_get_len(handler) <= index){
        log_error("index over current len, can not get node\n");
        return TANK_FAIL;
    }
    list_node_t *node = handler->head;

    while(index > 0){
        node = node->next;
        index -= 1;
    }
    memcpy(node->data, nex_data, handler->data_size);
    return TANK_SUCCESS;
}
tank_status_t list_get_node(list_head_t *handler, uint16_t index, void *data)
{
    if(list_is_empty(handler)){
        log_error("list is empty, can not get node\n");
        return TANK_FAIL;
    }
    if(list_get_len(handler) <= index){
        log_error("index over current len, can not get node\n");
        return TANK_FAIL;
    }
    list_node_t *node = handler->head;

    while(index > 0){
        node = node->next;
        index -= 1;
    }
    memcpy(data, node->data, handler->data_size);
    return TANK_SUCCESS;
}

tank_status_t list_delete_node(list_head_t *handler, uint16_t index)
{
    if(list_is_empty(handler)){
        log_error("list is empty, can not delete node\n");
        return TANK_FAIL;
    }
    if(list_get_len(handler) <= index){
        log_error("index over current len, can not delete node\n");
        return TANK_FAIL;
    }
    list_node_t *node = handler->head;
    list_node_t *last_node = node;

    if(index == 0){
        handler->head = node->next;
    }else{
        while(index > 0){
            last_node = node;
            node = node->next;
            index -= 1;
        }
        last_node->next = node->next;
    }
    tank_mm_free(handler->mm, node);
    handler->cur_len -= 1;
    return TANK_SUCCESS;
}

list_node_t *list_get_last_node(list_head_t *handler)
{
    list_node_t *node = handler->head;
    if(node == NULL){
        return NULL;
    }
    while(node->next != NULL){
        node = node->next;
    }
    return node;
}

tank_status_t list_is_full(list_head_t *handler)
{
    if(handler->cur_len == handler->max_len){
        return TANK_SUCCESS;
    }
    return TANK_FAIL;
}

tank_status_t list_is_empty(list_head_t *handler)
{
    if(handler->cur_len == 0){
        return TANK_SUCCESS;
    }
    return TANK_FAIL;
}

uint16_t list_get_len(list_head_t *handler)
{
    return handler->cur_len;
}