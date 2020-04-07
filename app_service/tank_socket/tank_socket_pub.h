#ifndef __TANK_SOCKET_PUB_H__
#define __TANK_SOCKET_PUB_H__

#include "tank_pub.h"
#include "tank_msgq.h"
#include "tank_mm.h"


typedef uint16_t port_t;
typedef uint32_t addr_t;
typedef uint16_t ts_id_t;
typedef uint16_t ts_msg_len_t;

#define TS_HOST_MAX 10
#define TS_NAME_SIZE_MAX 8

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
}ts_addr_t;

typedef enum{
    TS_STREAM = 0,
    TS_PAKCKET
}ts_type_t;

typedef enum{
    TS_TCP = 0,
    TS_UDP,
    TS_IP,
    TS_ICMP,
    TS_PROTO_LEN
}ts_protocol_t;


typedef enum{
    TS_ACCEPT_WAIT = 0,
    TS_ACCEPT_NOWAIT
}ts_accept_type_t;


typedef struct{
    union{
        tcp_state_t state;
    };
}ts_connect_status_t;

// typedef struct{
//     ts_msg_len_t len;
//     uint8_t *buf;
// }ts_msg_t;

// typedef struct ts_msg_node{
//     ts_msg_len_t len;
//     void *buf;
//     struct ts_msg_node *next;
// }ts_msg_node_t;

typedef struct{
    ts_id_t             id;
    ts_addr_t           addr;
    ts_protocol_t       protocol;
    ts_type_t           type;
    char                name[TS_NAME_SIZE_MAX];
    ts_connect_status_t connect_status[TS_HOST_MAX];
    tank_mm_t           mm_handler;
    tank_msgq_t         *sender;
    tank_msgq_t         *receiver;
    tank_msgq_t         *packet;
}ts_info_t;


ts_id_t         ts_socket_creat(ts_protocol_t protocol, ts_type_t type);
tank_status_t     ts_connect(ts_id_t id, ts_addr_t addr);
tank_status_t     ts_bind(ts_id_t id, ts_addr_t addr);
tank_status_t     ts_listen(ts_id_t id, uint16_t max_len);
ts_id_t         ts_accept(ts_id_t id, ts_addr_t *client_addr, ts_accept_type_t type);
ts_msg_len_t    ts_read(ts_id_t id, const char *buf, ts_msg_len_t max_len);
ts_msg_len_t    ts_write(ts_id_t id, char *buf, ts_msg_len_t max_len);


#endif