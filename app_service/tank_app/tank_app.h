#ifndef __TANK_APP_H__
#define __TANK_APP_H__

#include "tank_app_pub.h"
#include "tank_request.h"


void *recv_thread(ta_info_t *ta);
void *send_package_thread(ta_info_t *ta);
tank_status_t tank_app_recv_all(ta_info_t *ta);

tank_status_t check_ta_id_exist(ta_info_t *ta, tank_id_t id);
tank_status_t tank_app_listen(ta_info_t *ta, tank_id_t id);

tank_status_t find_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t *state);
tank_status_t write_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t state);


tank_status_t tank_app_creat(ta_info_t *ta, tank_id_t id, ta_protocol_t protocol, ta_type_t type);
tank_status_t tank_app_send_msg(ta_info_t *ta, tank_id_t dst_id, tcp_header_flag_t flag);


tank_status_t tank_app_recv_msg(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag);
tank_status_t tank_app_recv_msg_wait(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag);


tank_status_t ta_send_package(ta_info_t *ta, tank_id_t dst_id, void *package, uint32_t size, uint32_t tiemout);


tank_status_t send_package_request(ta_info_t *ta, uint16_t index);
tank_status_t get_package_allocate(ta_info_t *ta, uint16_t index, app_request_info_t *info);
tank_status_t send_package_finished(ta_info_t *ta, uint16_t index);
// ta_id_t         ta_socket_creat(ta_protocol_t protocol, ta_type_t type);
// tank_status_t     ta_connect(ta_id_t id, ta_addr_t addr);
// tank_status_t     ta_bind(ta_id_t id, ta_addr_t addr);
// tank_status_t     ta_listen(ta_id_t id, uint16_t max_len);
// ta_id_t         ta_accept(ta_id_t id, ta_addr_t *client_addr, ta_accept_type_t type);
// ta_msg_len_t    ta_read(ta_id_t id, const char *buf, ta_msg_len_t max_len);
// ta_msg_len_t    ta_write(ta_id_t id, char *buf, ta_msg_len_t max_len);


#endif