#include "tank_mm.h"

tank_status_t tank_mm_register(tank_mm_t *handler, uint32_t addr, uint32_t size, const char *name)
{
    strncpy(handler->name, name, 256);
    handler->heap.addr = addr;
    handler->heap.total_size = size;
    handler->heap.pxEnd = NULL;
    handler->heap.xFreeBytesRemaining = 0;
    handler->heap.xMinimumEverFreeBytesRemaining = 0;
    my_sem_creat(&handler->sem, 1);
    return TANK_SUCCESS;
}

void *tank_mm_malloc(tank_mm_t *handler, uint32_t size)
{
    void *p = NULL;
    my_sem_wait(&handler->sem);
    p = pvPortMalloc(&handler->heap, size);
    my_sem_post(&handler->sem);
    return p;
}


void *tank_mm_alloc(tank_mm_t *handler, uint32_t size)
{
    void *p = NULL;
    my_sem_wait(&handler->sem);
    p = pvPortMalloc(&handler->heap, size);
    if(p != NULL){
        memset(p, 0, size);
    }
    my_sem_post(&handler->sem);
    return p;
}


void tank_mm_free(tank_mm_t *handler, void *addr)
{
    my_sem_wait(&handler->sem);
    vPortFree(&handler->heap, addr);
    my_sem_post(&handler->sem);
}