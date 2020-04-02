#ifndef __HEAP_4_H__
#define __HEAP_4_H__

#include "tank_pub.h"
/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

typedef struct{
    BlockLink_t *xStart;
    BlockLink_t *pxEnd;
    uint32_t xFreeBytesRemaining;
    uint32_t xMinimumEverFreeBytesRemaining;
    uint32_t addr;
    uint32_t total_size;
}heap_info_t;

void *pvPortMalloc(heap_info_t *info, uint32_t xWantedSize);
void vPortFree(heap_info_t *info, void *pv);
#endif