#ifndef __TANK_MAP_H__
#define __TANK_MAP_H__
#include "tank_pub.h"
#include "my_sem.h"

#define TANK_PUB_NAME ("/tank_pub")
#define TANK_PUB_SIZE (uint32_t)(64*1024)
#define MSGQ_ALL_SIZE (uint32_t)(16*1024)
#define SEM_ADDR (4)
#define MALLOC_ADDR (SEM_ADDR + sizeof(my_sem_t))
#define TANK_MSGQ_BASE (128)


#endif