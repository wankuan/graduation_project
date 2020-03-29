#include "tank_socket.h"


ts_id_t ts_socket(ts_protocol_t protocol, ts_type_t type)
{
    
}
ts_status_t     ts_connect(ts_id_t id, ts_addr_t addr);
ts_status_t     ts_bind(ts_id_t id, ts_addr_t addr);
ts_status_t     ts_listen(ts_id_t id, uint16_t max_len);
ts_id_t         ts_accept(ts_id_t id, ts_addr_t *client_addr, ts_accept_type_t type);

ts_msg_len_t    ts_read(ts_id_t id, char *buf, ts_msg_len_t len);
ts_msg_len_t    ts_recvmsg(ts_id_t id, char *buf, ts_msg_len_t len, ts_msg_priority_t *group);

ts_msg_len_t    ts_write(ts_id_t id, const char *buf, ts_msg_len_t len);
ts_msg_len_t    ts_sendmsg(ts_id_t id, const char *buf, ts_msg_len_t len, ts_msg_priority_t group);

ts_msg_len_t    ts_recvfrom(ts_id_t id, char *buf, ts_msg_len_t len, ts_addr_t addr);
ts_msg_len_t    ts_sendto(ts_id_t id, const char *buf, ts_msg_len_t len, ts_addr_t addr);