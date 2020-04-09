#ifndef __TANK_APP_PUB_H__
#define __TANK_APP_PUB_H__

#include "tank_pub.h"
#include "tank_msgq.h"
#include "tank_mm.h"


typedef uint16_t port_t;
typedef uint32_t addr_t;
typedef uint16_t ta_id_t;
typedef uint16_t ta_msg_len_t;

#define TA_HOST_MAX 10
#define TA_NAME_SIZE_MAX 8

typedef enum {
    CLOSED      = 0,
    LISTEN      = 1,
    SYN_SENT    = 2,
    SYN_RCVD    = 3,
    ESTABLISHED = 4,
    FIN_WAIT_1  = 5,
    FIN_WAIT_2  = 6,
    CLOSE_WAIT  = 7,
    CLOSING     = 8,
    LAST_ACK    = 9,
    TIME_WAIT   = 10
}tcp_state_t;

typedef struct{
    addr_t addr;
    port_t port;
}ta_addr_t;

typedef enum{
    TA_STREAM = 0,
    TA_PAKCKET
}ta_type_t;

typedef enum{
    TA_TCP = 0,
    TA_UDP,
    TA_IP,
    TA_ICMP,
    TA_PROTO_LEN
}ta_protocol_t;


typedef enum{
    TA_ACCEPT_WAIT = 0,
    TA_ACCEPT_NOWAIT
}ta_accept_type_t;


typedef struct{
    union{
        tcp_state_t state;
    };
}ta_connect_status_t;

typedef struct{
    ta_id_t             id;
    ta_addr_t           addr;
    ta_protocol_t       protocol;
    ta_type_t           type;
    char                name[TA_NAME_SIZE_MAX];
    ta_connect_status_t connect_status[TA_HOST_MAX];
    tank_mm_t           mm_handler;
    tank_msgq_t         *sender;
    tank_msgq_t         *receiver;
    tank_msgq_t         *packet;
}ta_info_t;


ta_id_t         ta_socket_creat(ta_protocol_t protocol, ta_type_t type);
tank_status_t     ta_connect(ta_id_t id, ta_addr_t addr);
tank_status_t     ta_bind(ta_id_t id, ta_addr_t addr);
tank_status_t     ta_listen(ta_id_t id, uint16_t max_len);
ta_id_t         ta_accept(ta_id_t id, ta_addr_t *client_addr, ta_accept_type_t type);
ta_msg_len_t    ta_read(ta_id_t id, const char *buf, ta_msg_len_t max_len);
ta_msg_len_t    ta_write(ta_id_t id, char *buf, ta_msg_len_t max_len);


#endif