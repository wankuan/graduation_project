#include "tcp_fsm.h"

#include "tank_log_api.h"
#define FILE_NAME "tcp_fsm"


tank_status_t do_something(void *arg)
{
    printf("fsm cb do something\n");
    return TANK_SUCCESS;
}

tank_status_t cb_invlid_id(void *arg)
{
    printf("dst id invlid\n");
    return TANK_SUCCESS;
}
tcp_fsm_transform_t transform_table[] = {
    {LISTEN, SYN_RCVD, TCP_SYN, TCP_SYN|TCP_ACK, do_something},
    {SYN_RCVD, ESTABLISHED, TCP_ACK, TCP_NON, do_something},
    {SYN_SENT, ESTABLISHED, TCP_SYN|TCP_ACK, TCP_ACK, do_something},
    {SYN_SENT, SYN_SENT, TCP_ID_INVALID, TCP_NON, cb_invlid_id},
    {ESTABLISHED, LAST_ACK, TCP_FIN, TCP_ACK|TCP_FIN, do_something},
    {LAST_ACK, CLOSED, TCP_ACK, TCP_NON, do_something},
    {FIN_WAIT_1, CLOSED, TCP_ACK|TCP_FIN, TCP_ACK, do_something},
};

tank_status_t find_fsm_table(tcp_state_t cur_state, tcp_state_t* next_state, tcp_header_flag_t recv_flag, tcp_header_flag_t *send_flag, tank_status_t (**cb)(void *))
{
    uint16_t size = sizeof(transform_table)/sizeof(transform_table[0]);
    for(uint16_t i=0;i<size;++i){
        if((recv_flag == transform_table[i].recv_flag) && (cur_state == transform_table[i].cur_state)){
            *next_state = transform_table[i].next_state;
            *send_flag = transform_table[i].send_flag;
            *cb = transform_table[i].action_cb;
            return TANK_SUCCESS;
        }
    }
    log_error("not find match state in fsm-table\n");
    return TANK_FAIL;
}
