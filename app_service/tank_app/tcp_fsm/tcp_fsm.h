#ifndef __TCP_FSM_H__
#define __TCP_FSM_H__

#include "tank_pub.h"
#include "tank_app_pub.h"
#include "tank_app.h"




typedef struct{
    tcp_state_t cur_state;
    tcp_state_t next_state;
    tcp_header_flag_t recv_flag;
    tcp_header_flag_t send_flag;
}tcp_fsm_transform_t;

tank_status_t tank_fsm_recv(ta_info_t *ta);

#endif