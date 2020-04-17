#ifndef __TANK_MM_H__
#define __TANK_MM_H__
#include "tank_pub.h"
#include "my_sem.h"
#include "heap_4.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

typedef struct{
    my_sem_t sem;
    char name[64];
    heap_info_t heap;
}tank_mm_t;

// tank_mm_t 一定要创建全局变量
tank_status_t tank_mm_register(tank_mm_t *handler, uint32_t addr, uint32_t size, const char *name);

void *tank_mm_malloc(tank_mm_t *handler, uint32_t size);

void tank_mm_free(tank_mm_t *handler, void *addr);

void *tank_mm_calloc(tank_mm_t *handler, uint32_t size);

#endif