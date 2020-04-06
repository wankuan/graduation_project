#ifndef __TANK_SOCKET_PUB_H__
#define __TANK_SOCKET_PUB_H__

#include "tank_pub.h"

typedef uint16_t port_t;
typedef uint32_t addr_t;
typedef uint16_t ts_id_t;
typedef uint16_t ts_msg_len_t;


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
    TS_SUCCESS = 0,
    TS_FAIL = 1
}tank_status_t;

typedef enum{
    TS_ACCEPT_WAIT = 0,
    TS_ACCEPT_NOWAIT
}ts_accept_type_t;


typedef struct{
    ts_msg_len_t len;
    uint8_t *buf;
}ts_msg_t;

typedef struct ts_msg_node{
    ts_msg_len_t len;
    void *buf;
    struct ts_msg_node *next;
}ts_msg_node_t;

typedef struct{
    ts_id_t id;
    ts_addr_t addr;
    ts_protocol_t protocol;
    ts_type_t type;
    char name[8];
    ts_msg_node_t msg_list;
}ts_info_t;


typedef struct{
    ts_id_t fd;
    union{
        ts_addr_t dst_addr;
        ts_addr_t src_addr;
    };
    ts_msg_node_t *send;
    ts_msg_node_t *recv;
}






ts_id_t         ts_socket_creat(ts_protocol_t protocol, ts_type_t type);
tank_status_t     ts_connect(ts_id_t id, ts_addr_t addr);
tank_status_t     ts_bind(ts_id_t id, ts_addr_t addr);
tank_status_t     ts_listen(ts_id_t id, uint16_t max_len);
ts_id_t         ts_accept(ts_id_t id, ts_addr_t *client_addr, ts_accept_type_t type);
ts_msg_len_t    ts_read(ts_id_t id, const char *buf, ts_msg_len_t max_len);
ts_msg_len_t    ts_write(ts_id_t id, char *buf, ts_msg_len_t max_len);


#endif