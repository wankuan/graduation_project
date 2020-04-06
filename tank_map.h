#ifndef __TANK_MAP_H__
#define __TANK_MAP_H__
#include "tank_pub.h"
#include "my_sem.h"


#define TANK_PUB_NAME       ("/tank_pub")
#define SHM_SIZE            (uint32_t)(64*1024)
#define INNER_SWAP_SIZE     (uint32_t)(32*1024)
#define APP_SIZE            (uint32_t)(16*1024)

#define ADDR_MAP_PREFIX     (uint32_t)(4)
#define SEM_PREFIX          (uint32_t)(128)


#define SHM_PREFIX          (uint32_t)(256)
#define APP_REQUEST_PREFIX (uint32_t)(512)

#define APP_PREFIX       (uint32_t)(MSGQ_SIZE)



#define MSGQ_MAP_ADDR            (uint32_t)(shm_base + ADDR_MAP_PREFIX)

#define SEM_ADDR                 (uint32_t)(shm_base + SEM_PREFIX)
#define TEMP_ADDR                 (uint32_t)(shm_base + 40)

#define INNER_SWAP_ADDR            (uint32_t)(shm_base + SHM_PREFIX)
#define APP_REQUEST_ADDR   (uint32_t)(shm_base + APP_REQUEST_PREFIX)

#define APP_ADDR         (uint32_t)(shm_base + APP_PREFIX)

extern uint32_t shm_base;

#endif