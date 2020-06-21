#ifndef __TANK_PUB_TYPE_H__
#define __TANK_PUB_TYPE_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEBUG_


#ifdef DEBUG_
#define debug_printf(fmt, ...)  printf("File:%s, Line:%04d, "fmt"",__FILE__,__LINE__, ##__VA_ARGS__)
#elif
#define debug_printf(fmt, ...)
#endif

// typedef enum _tank_status_t{
//     TANK_SUCCESS = 0,
//     TANK_FAIL = 1
// }tank_status_t;

#endif



