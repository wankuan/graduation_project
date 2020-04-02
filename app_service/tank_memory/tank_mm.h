#ifndef __TANK_MM_H__
#define __TANK_MM_H__
#include "my_sem.h"
#include "heap_4.h"


/* Create a couple of list links to mark the start and end of the list. */
// static BlockLink_t xStart, *pxEnd = NULL;

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = 0;


typedef struct{
    my_sem_t sem;
    char name[16];
    heap_info_t heap;
}tank_mm_t;


tank_status_t tank_mm_register(tank_mm_t *handler, uint32_t addr, uint32_t size, const char *name);

void *tank_mm_malloc(tank_mm_t *handler, uint32_t size);

void tank_mm_free(tank_mm_t *handler, void *addr);

#endif