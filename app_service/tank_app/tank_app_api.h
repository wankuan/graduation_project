#ifndef __TANK_APP_API_H__
#define __TANK_APP_API_H__

#include "tank_app_pub.h"
#include "tank_request.h"

#define __weak __attribute__((weak))


tank_status_t tank_app_creat(ta_info_t *ta, tank_id_t id, ta_protocol_t protocol, ta_type_t type, recv_type_t recv_type);

tank_status_t ta_send_package(ta_info_t *ta, tank_id_t dst_id, void *package, uint32_t size, uint32_t tiemout);
tank_status_t ta_recv_package(ta_info_t *ta, tank_id_t *src_id, void* packgae, uint16_t *size, uint16_t oversize);

#endif