#ifndef __TANK_SOCKET_H__
#define __TANK_SOCKET_H__

#include "tank_socket_pub.h"


tank_status_t tank_socket_creat(ts_info_t *ts, const char * name, ts_protocol_t protocol, ts_type_t type);



// ts_id_t         ts_socket_creat(ts_protocol_t protocol, ts_type_t type);
// tank_status_t     ts_connect(ts_id_t id, ts_addr_t addr);
// tank_status_t     ts_bind(ts_id_t id, ts_addr_t addr);
// tank_status_t     ts_listen(ts_id_t id, uint16_t max_len);
// ts_id_t         ts_accept(ts_id_t id, ts_addr_t *client_addr, ts_accept_type_t type);
// ts_msg_len_t    ts_read(ts_id_t id, const char *buf, ts_msg_len_t max_len);
// ts_msg_len_t    ts_write(ts_id_t id, char *buf, ts_msg_len_t max_len);


#endif