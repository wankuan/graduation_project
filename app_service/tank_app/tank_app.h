#ifndef __TANK_APP_H__
#define __TANK_APP_H__

#include "tank_app_pub.h"
#include "tank_request.h"

#define __weak __attribute__((weak))


void *recv_fun(void *arg);
void *send_fun(void *arg);
tank_status_t tank_app_recv_all(ta_info_t *ta);

tank_status_t check_ta_id_exist(ta_info_t *ta, tank_id_t id);
tank_status_t tank_app_listen(ta_info_t *ta, tank_id_t id);

tank_status_t find_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t *state);
tank_status_t write_tcp_state(ta_info_t *ta, tank_id_t id, tcp_state_t state);





tank_status_t tank_app_destory(ta_info_t *ta);


tank_status_t tank_app_recv_package_callback(app_package_info_t* info);

tank_status_t tank_app_tcp_send(ta_info_t *ta, tank_id_t dst_id, tcp_header_flag_t flag);


tank_status_t tank_app_recv_msg(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag);
tank_status_t tank_app_recv_msg_wait(ta_info_t *ta, tank_id_t *src_id, tcp_header_flag_t *flag);

tank_status_t send_package_request(ta_info_t *ta, uint16_t index);
tank_status_t get_package_allocate(ta_info_t *ta, uint16_t index, app_request_info_t *info);
tank_status_t send_package_finished(ta_info_t *ta, uint16_t index);

tank_status_t recv_package_ack(ta_info_t *ta, uint32_t package_id);

#endif