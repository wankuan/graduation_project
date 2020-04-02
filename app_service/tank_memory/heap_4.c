/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#include "heap_4.h"


/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const uint32_t xHeapStructSize = (sizeof(BlockLink_t) + ((uint32_t)(portBYTE_ALIGNMENT - 1 ))) &~ (( uint32_t)portBYTE_ALIGNMENT_MASK);
// 把最高位作为是否分配标志位，若分配malloc则为1，free后为0
/* Work out the position of the top bit in a uint32_t variable. */
static uint32_t xBlockAllocatedBit = ((uint32_t)1) << 31;

/* Block sizes must not get too small. */
// 用于块分配，必须大于两倍的头结构
#define heapMINIMUM_BLOCK_SIZE	((uint32_t)(xHeapStructSize << 1))

/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList(heap_info_t *heap, BlockLink_t *pxBlockToInsert );

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit(heap_info_t *heap);

/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

void *pvPortMalloc(heap_info_t *heap, uint32_t xWantedSize )
{
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void *pvReturn = NULL;

    /* If this is the first call to malloc then the heap will require
    initialisation to setup the list of free blocks. */
    if( heap->pxEnd == NULL )
    {
        printf("[INFO]first use, heap init\n");
        prvHeapInit(heap);
    }

    /* Check the requested block size is not so large that the top bit is
    set.  The top bit of the block size member of the BlockLink_t structure
    is used to determine who owns the block - the application or the
    kernel, so it must be free. */
    if( ( xWantedSize & xBlockAllocatedBit ) == 0 )
    {
        /* The wanted size is increased so it can contain a BlockLink_t
        structure in addition to the requested amount of bytes. */
        if( xWantedSize > 0 )
        {
            xWantedSize += xHeapStructSize;
            /* Ensure that blocks are always aligned to the required number
            of bytes. */
            if( ( xWantedSize & portBYTE_ALIGNMENT_MASK ) != 0x00 )
            {
                /* Byte alignment required. */
                // 往大的分配，保证是8的倍数
                xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
                printf("[INFO]the size not reach 8 mask\n");
            }
        }
        if( ( xWantedSize > 0 ) && ( xWantedSize <= heap->xFreeBytesRemaining ) )
        {
            /* Traverse the list from the start	(lowest address) block until
            one	of adequate size is found. */
            pxPreviousBlock = &heap->xStart;
            pxBlock = heap->xStart.pxNextFreeBlock;
            // 寻找足够大的空闲块，如果找到或者最后一个空闲块会退出
            while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
            {
                pxPreviousBlock = pxBlock;
                pxBlock = pxBlock->pxNextFreeBlock;
            }
            /* If the end marker was reached then a block of adequate size
            was	not found. */
            if( pxBlock != heap->pxEnd )
            {
                /* Return the memory space pointed to - jumping over the
                BlockLink_t structure at its start. */
                pvReturn = (void *)(((uint8_t *)pxPreviousBlock->pxNextFreeBlock) + xHeapStructSize);

                /* This block is being returned for use so must be taken out
                of the list of free blocks. */
                pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                /* If the block is larger than required it can be split into
                two. */
                if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
                {
                    /* This block is to be split into two.  Create a new
                    block following the number of bytes requested. The void
                    cast is used to prevent byte alignment warnings from the
                    compiler. */
                    pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + xWantedSize );
                    // configASSERT( ( ( ( uint32_t ) pxNewBlockLink ) & portBYTE_ALIGNMENT_MASK ) == 0 );

                    /* Calculate the sizes of two blocks split from the
                    single block. */
                    pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                    pxBlock->xBlockSize = xWantedSize;

                    /* Insert the new block into the list of free blocks. */
                    prvInsertBlockIntoFreeList(heap, pxNewBlockLink);
                }
                else
                {
                    printf("[INFO]small block appear, can not split, size:%d\n", pxBlock->xBlockSize - xWantedSize);
                }

                heap->xFreeBytesRemaining -= pxBlock->xBlockSize;

                if( heap->xFreeBytesRemaining < heap->xMinimumEverFreeBytesRemaining )
                {
                    heap->xMinimumEverFreeBytesRemaining = heap->xFreeBytesRemaining;
                    printf("[INFO]appear new minum free byte remainning, size:%d\n", heap->xMinimumEverFreeBytesRemaining);
                }
                else
                {
                    printf("[INFO]has allocated remain_size:%d\n", heap->xFreeBytesRemaining);
                }

                /* The block is being returned - it is allocated and owned
                by the application and has no "next" block. */
                // 将最高位标记为1 表示该队列已分配
                pxBlock->xBlockSize |= xBlockAllocatedBit;
                pxBlock->pxNextFreeBlock = NULL;
            }
            else
            {
                printf("[ERROR]reach end block %d\n", heap->xFreeBytesRemaining);
                return NULL;
            }
        }
        else
        {
            printf("[ERROR]size over remain %d\n", heap->xFreeBytesRemaining);
            return NULL;
        }
    }
    else
    {
        printf("[ERROR]size over max %d\n", xBlockAllocatedBit);
        return NULL;
    }
    printf("[INFO]allocated OK return:%p, size:%d\n", pvReturn, xWantedSize);
    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree(heap_info_t *heap, void *pv)
{
    uint8_t *puc = ( uint8_t * ) pv;
    BlockLink_t *pxLink;
    printf("[INFO]free start addr:%p\n", puc);
    if( pv != NULL )
    {
        /* The memory being freed will have an BlockLink_t structure immediately
        before it. */
        puc -= xHeapStructSize;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = ( void * ) puc;

        /* Check the block is actually allocated. */
        if((pxLink->xBlockSize & xBlockAllocatedBit )==0)
        {
            printf("[ERROR]the memory not to allocated\n");
        }
        if( pxLink->pxNextFreeBlock != NULL )
        {
            printf("[ERROR]the block next not a NULL\n");
        }

        if( ( pxLink->xBlockSize & xBlockAllocatedBit ) != 0 )
        {
            if( pxLink->pxNextFreeBlock == NULL )
            {
                /* The block is being returned to the heap - it is no longer
                allocated. */
                // 清除最高位的已分配标志位
                pxLink->xBlockSize &= ~xBlockAllocatedBit;

                /* Add this block to the list of free blocks. */
                heap->xFreeBytesRemaining += pxLink->xBlockSize;

                // 空闲块插入
                prvInsertBlockIntoFreeList(heap, (BlockLink_t *)pxLink);
            }
            else
            {
                printf("[ERROR]next block not NULL \n");
            }
        }
        else
        {
            printf("[ERROR]size over max %d\n", xBlockAllocatedBit);
        }
    }
    printf("[INFO]free OK\n");
}
/*-----------------------------------------------------------*/

uint32_t xPortGetFreeHeapSize(heap_info_t *heap)
{
    printf("[INFO]xFreeBytesRemaining:%d\n", heap->xFreeBytesRemaining);
    return heap->xFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

uint32_t xPortGetMinimumEverFreeHeapSize( heap_info_t *heap )
{
    printf("[INFO]xMinimumEverFreeBytesRemaining:%d\n", heap->xMinimumEverFreeBytesRemaining);
    return heap->xMinimumEverFreeBytesRemaining;
}
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

static void prvHeapInit(heap_info_t *heap)
{
    BlockLink_t *pxFirstFreeBlock;
    uint8_t *pucAlignedHeap;
    uint32_t uxAddress;
    uint32_t xTotalHeapSize = heap->total_size;
    printf("[INFO]start init heap\n");
    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (uint32_t)heap->addr;
    // 字节对齐  如果首地址不是8字节对齐
    if( ( uxAddress & portBYTE_ALIGNMENT_MASK ) != 0 )
    {
        uxAddress += ( portBYTE_ALIGNMENT - 1 );
        uxAddress &= ~( ( uint32_t ) portBYTE_ALIGNMENT_MASK );
        xTotalHeapSize -= uxAddress - (uint32_t)heap->addr;
    }
    //获得对齐后的地址
    pucAlignedHeap = ( uint8_t * ) uxAddress;

    /* xStart is used to hold a pointer to the first item in the list of free
    blocks.  The void cast is used to prevent compiler warnings. */
    // 记录首个块的地址
    heap->xStart.pxNextFreeBlock = ( void * ) pucAlignedHeap;
    heap->xStart.xBlockSize = ( uint32_t ) 0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    uxAddress = ( ( uint32_t ) pucAlignedHeap ) + xTotalHeapSize;
    uxAddress -= xHeapStructSize;
    uxAddress &= ~( ( uint32_t ) portBYTE_ALIGNMENT_MASK );
    heap->pxEnd = ( void * ) uxAddress;
    heap->pxEnd->xBlockSize = 0;
    heap->pxEnd->pxNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
    entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = ( void * ) pucAlignedHeap;
    pxFirstFreeBlock->xBlockSize = uxAddress - ( uint32_t ) pxFirstFreeBlock;
    pxFirstFreeBlock->pxNextFreeBlock = heap->pxEnd;

    /* Only one block exists - and it covers the entire usable heap space. */
    heap->xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    heap->xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    printf("xHeapStructSize is %d\n", xHeapStructSize);
    printf("xFreeBytesRemaining is %d\n", heap->xFreeBytesRemaining );
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList(heap_info_t *heap, BlockLink_t *pxBlockToInsert )
{
    BlockLink_t *pxIterator;
    uint8_t *puc;
    printf("[INFO]start prvInsertBlockIntoFreeList\n");
    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    // 寻找当前要插入的地址位置，空闲块在物理地址上从小到大，插入
    for( pxIterator = &heap->xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
    make a contiguous block of memory? */
    puc = ( uint8_t * ) pxIterator;
    // 检查和上一个地址是否连续，如果连续，则插入
    if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
        printf("[INFO]the addr continue for last\n");
    }
    else
    {
        printf("[INFO]the addr not continue for last\n");
    }

    /* Do the block being inserted, and the block it is being inserted before
    make a contiguous block of memory? */
    puc = ( uint8_t * ) pxBlockToInsert;
    // 检查和下一个地址是否连续，如果连续，则插入
    if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
    {
        if( pxIterator->pxNextFreeBlock != heap->pxEnd )
        {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = heap->pxEnd;
        }
        printf("[INFO]the addr continue for next\n");
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
        printf("[INFO]the addr not continue for next\n");
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pxNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    //如果全部都没有合并，迭代器的空闲快不等于插入的
    if( pxIterator != pxBlockToInsert )
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
        printf("[INFO]the addr not continue for last/next\n");
    }
    printf("[INFO]end prvInsertBlockIntoFreeList\n");
}

