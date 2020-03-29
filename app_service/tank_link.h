#ifndef __TANK_LINK_H__
#define __TANK_LINK_H__
#include "tank_app.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include "lib_pub.h"
#include <errno.h>

typedef enum{
    TRANSMIT_IDLE = 0,
    TRANSMIT_REQUEST = 1,
    TRANSMIT_ACK_INFO = 2,
    TRANSMIT_SEND = 3,
    TRANSMIT_FINISHED = 4
}TRANSMIT_STATUS;

typedef enum{
    LINK_FAIL = 0,
    LINK_SUCCESS = 1
}LINK_STATUS;

typedef TRANSMIT_STATUS transmit_status_t;
typedef LINK_STATUS link_status_t;

typedef struct {
    transmit_status_t transmit_status;

    app_packet_t* app_packet;
}link_packet_t;


typedef struct {
    link_status_t (*link_request)(system_id_t src, system_id_t dst, size_t size);
    link_status_t (*link_get_ack_info)(void);
    link_status_t (*link_send_out)(void);
    link_status_t (*link_finished)(void);
}link_handler_t;

typedef APP_STATUS app_status_t;

transmit_status_t get_packet_status(app_packet_t *app_packet);
link_status_t transmit_packet(app_packet_t *app_packet);


extern link_handler_t my_link_handler;


typedef struct _my_msg_t {
    long type;
    char string[100];
} my_msg_t;
// init link_handler_t pointer
link_status_t tank_link_init(void);


int tank_link_msg_send(void);
int tank_link_msg_rev(void);


#endif