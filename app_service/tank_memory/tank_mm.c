#include "tank_mm.h"

tank_status_t tank_mm_register(tank_mm_t *handler, uint32_t addr, uint32_t size, const char *name)
{
    strncpy(handler->name, name, 256);
    handler->heap.addr = addr;
    handler->heap.total_size = size;
    handler->heap.pxEnd = NULL;
    handler->heap.xFreeBytesRemaining = 0;
    handler->heap.xMinimumEverFreeBytesRemaining = 0;
}

void *tank_mm_malloc(tank_mm_t *handler, uint32_t size)
{
    return pvPortMalloc(&handler->heap, size);
}


void *tank_mm_alloc(tank_mm_t *handler, uint32_t size)
{
    void *p = pvPortMalloc(&handler->heap, size);
    memset(p, 0, size);
    return p;
}


void tank_mm_free(tank_mm_t *handler, void *addr)
{
    return vPortFree(&handler->heap, addr);
}