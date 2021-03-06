#ifndef __TANK_PUB_H__
#define __TANK_PUB_H__
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define __LINUX__

#define TANK_MAX_SIZE 256

typedef uint16_t tank_queue_id_t;
typedef uint16_t tank_id_t;

typedef enum{
    TANK_SUCCESS = 0,
    TANK_FAIL = 1
}tank_status_t;


#endif