#ifndef __TANK_MAP_H__
#define __TANK_MAP_H__
#include "tank_pub.h"
#include "tank_msgq.h"
#include "my_sem.h"

#ifdef __ANDROID__
#define TANK_PUB_NAME       ("/data/tank_pub")
#else
#define TANK_PUB_NAME       ("/home/wankuan/tank_pub")
#endif

#define SHM_SIZE            (uint32_t)(64*1024)
#define INNER_SWAP_SIZE     (uint32_t)(32*1024)
#define APP_SIZE            (uint32_t)(16*1024)

#define ADDR_MAP_PREFIX     (uint32_t)(64)
#define SEM_PREFIX          (uint32_t)(4)


#define SHM_PREFIX          (uint32_t)(256)
#define APP_REQUEST_PREFIX (uint32_t)(512)

#define APP_PREFIX       (uint32_t)(MSGQ_SIZE)

#define MSGQ_MAP_ADDR            (uint32_t)(g_shm_base + ADDR_MAP_PREFIX)

#define SEM_ADDR                 (uint32_t)(g_shm_base + SEM_PREFIX)

#define INNER_SWAP_ADDR            (uint32_t)(g_shm_base + SHM_PREFIX)
#define APP_REQUEST_ADDR   (uint32_t)(g_shm_base + APP_REQUEST_PREFIX)

#define APP_ADDR         (uint32_t)(g_shm_base + APP_PREFIX)

extern volatile uint32_t g_shm_base;

tank_status_t tank_creat_shm(void);
tank_status_t get_shm_base_addr(void);
tank_status_t get_service_msgq_addr(tank_msgq_t **addr);


#endif