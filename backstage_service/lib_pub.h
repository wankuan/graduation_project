#ifndef __LIB_PUB_H__
#define __LIB_PUB_H__
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef uint32_t len_app_packet_t;

typedef enum{
    BYTE = 0,
    HALFWORD = 1,
    WORD = 2
}msg_type_t;

#endif