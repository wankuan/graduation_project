#ifndef __DATA_PACKET_H__
#define __DATA_PACKET_H__
#include "lib_pub.h"

// #define  USE_PACKED  (__attribute__((packed)))

typedef enum{
    SYSTEM_A = 0x01,
    SYSTEM_B = 0x02,
    SYSTEM_C = 0x03,
    SYSTEM_D = 0x04,
    SYSTEM_E = 0x05,
}systemID_t;

typedef struct{
    systemID_t ID_system_src;
    systemID_t ID_system_dst;
    len_app_packet_t len;
    uint8_t data[0];
}app_packet_t;







#endif