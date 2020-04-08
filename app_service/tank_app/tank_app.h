#ifndef __TANK_APP_H__
#define __TANK_APP_H__

#include "tank_app_pub.h"


tank_status_t tank_app_creat(ta_info_t *ta, const char * name, ta_protocol_t protocol, ta_type_t type);



// ta_id_t         ta_socket_creat(ta_protocol_t protocol, ta_type_t type);
// tank_status_t     ta_connect(ta_id_t id, ta_addr_t addr);
// tank_status_t     ta_bind(ta_id_t id, ta_addr_t addr);
// tank_status_t     ta_listen(ta_id_t id, uint16_t max_len);
// ta_id_t         ta_accept(ta_id_t id, ta_addr_t *client_addr, ta_accept_type_t type);
// ta_msg_len_t    ta_read(ta_id_t id, const char *buf, ta_msg_len_t max_len);
// ta_msg_len_t    ta_write(ta_id_t id, char *buf, ta_msg_len_t max_len);


#endif