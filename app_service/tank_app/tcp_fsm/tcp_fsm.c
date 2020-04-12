#include "tcp_fsm.h"

#include "tank_log_api.h"
#define FILE_NAME "tcp_fsm"


static tank_status_t find_fsm_table(tcp_state_t cur_state, tcp_state_t* next_state, tcp_header_flag_t recv_flag, tcp_header_flag_t *send_flag);

tcp_fsm_transform_t transform_table[] = {
    {LISTEN, SYN_RCVD, TCP_SYN, TCP_SYN|TCP_ACK},
    {SYN_RCVD, ESTABLISHED, TCP_ACK, TCP_NON},
    {SYN_SENT, ESTABLISHED, TCP_SYN|TCP_ACK, TCP_ACK},

    {ESTABLISHED, LAST_ACK, TCP_FIN, TCP_ACK|TCP_FIN},
    {LAST_ACK, CLOSED, TCP_ACK, TCP_NON},
    {FIN_WAIT_1, CLOSED, TCP_ACK|TCP_FIN, TCP_ACK},
};

static tank_status_t find_fsm_table(tcp_state_t cur_state, tcp_state_t* next_state, tcp_header_flag_t recv_flag, tcp_header_flag_t *send_flag)
{
    uint16_t size = sizeof(transform_table)/sizeof(transform_table[0]);
    for(uint16_t i=0;i<size;++i){
        if((recv_flag == transform_table[i].recv_flag) && (cur_state == transform_table[i].cur_state)){
            *next_state = transform_table[i].next_state;
            *send_flag = transform_table[i].send_flag;
            return TANK_SUCCESS;
        }
    }
    return TANK_FAIL;
}
static uint32_t fsm_cnt = 0;
tank_status_t tank_fsm_recv(ta_info_t *ta)
{
    tank_id_t src_id = 0;
    tcp_state_t cur_state = 0;
    tcp_state_t next_state = 0;
    tcp_header_flag_t recv_flag = 0;
    tcp_header_flag_t send_flag = 0;
    fsm_cnt ++;
    log_info("fsm running cnt:%d\n",fsm_cnt);

    tank_app_recv_wait(ta, &src_id, &recv_flag);

    if(find_tcp_state(ta, src_id, &cur_state) == TANK_FAIL){
        return TANK_FAIL;
    }
    if(find_fsm_table(cur_state, &next_state, recv_flag, &send_flag)){
        return TANK_FAIL;
    }
    if(write_tcp_state(ta, src_id, next_state)){
        return TANK_FAIL;
    }
    if(send_flag != TCP_NON){
        tank_app_send(ta, src_id, send_flag);
    }else{
        log_info("unnecessary to send\n");
    }
    log_info("fsm exit\n");
    return TANK_SUCCESS;
}