#ifndef __TANK_LINK_H__
#define __TANK_LINK_H__

#include <sys/ipc.h>
#include <sys/msg.h>
#include "lib_pub.h"
#include <errno.h>
typedef struct _my_msg_t {
    uint32_t type;
    char string[100];
} my_msg_t;
int tank_link_msg_send(void);
int tank_link_msg_rev(void);


#endif