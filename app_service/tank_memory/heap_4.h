#ifndef __HEAP_4_H__
#define __HEAP_4_H__

#include "tank_pub.h"

#define portBYTE_ALIGNMENT ((uint32_t)8)
#define portBYTE_ALIGNMENT_MASK ((uint32_t)(portBYTE_ALIGNMENT-1))

/* 分配原则  end结构8字节+N*(malloc大小+8) */

typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

typedef struct{
    BlockLink_t xStart;
    BlockLink_t *pxEnd;
    uint32_t    xFreeBytesRemaining;
    uint32_t    xMinimumEverFreeBytesRemaining;
    uint32_t    addr;
    uint32_t    total_size;
}heap_info_t;

void     *pvPortMalloc(heap_info_t *heap, uint32_t xWantedSize);
void     vPortFree(heap_info_t *heap, void *pv);
uint32_t xPortGetFreeHeapSize(heap_info_t *heap);
uint32_t xPortGetMinimumEverFreeHeapSize( heap_info_t *heap );

#endif