#ifndef __TANK_PUB_H__
#define __TANK_PUB_H__
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define __LINUX__
#define TANK_DEBUG
#define TANK_MAX_SIZE 256

#define TANK_MSGQ_NORMAL_SIZE 20
#define TANK_MSGQ_NORMAL_LEN 5

typedef uint16_t tank_queue_id_t;
typedef uint16_t tank_id_t;

#define SYS_ID_MASK (0x0100)
#define APP_ID_MASK (0x00FF)
#define GET_APP_ID(id) (id&APP_ID_MASK)
#define GET_SYS_ID(id) (id&SYS_ID_MASK)

typedef enum{
    TANK_SUCCESS = 1,
    TANK_FAIL = 0
}tank_status_t;


#endif