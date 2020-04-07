#ifndef __TANK_MAP_H__
#define __TANK_MAP_H__
#include "tank_pub.h"
#include "tank_msgq.h"
#include "my_sem.h"


#define TANK_PUB_NAME       ("/tank_pub")
#define SHM_SIZE            (uint32_t)(64*1024)
#define INNER_SWAP_SIZE     (uint32_t)(32*1024)
#define APP_SIZE            (uint32_t)(16*1024)

#define ADDR_MAP_PREFIX     (uint32_t)(64)
#define SEM_PREFIX          (uint32_t)(4)


#define SHM_PREFIX          (uint32_t)(256)
#define APP_REQUEST_PREFIX (uint32_t)(512)

#define APP_PREFIX       (uint32_t)(MSGQ_SIZE)

#define MSGQ_MAP_ADDR            (uint32_t)(shm_base_s + ADDR_MAP_PREFIX)

#define SEM_ADDR                 (uint32_t)(shm_base_s + SEM_PREFIX)

#define INNER_SWAP_ADDR            (uint32_t)(shm_base_s + SHM_PREFIX)
#define APP_REQUEST_ADDR   (uint32_t)(shm_base_s + APP_REQUEST_PREFIX)

#define APP_ADDR         (uint32_t)(shm_base_s + APP_PREFIX)

extern uint32_t shm_base_s;


tank_status_t get_service_base_addr(void);
tank_status_t get_service_msgq_addr(tank_msgq_t **addr);


#endif