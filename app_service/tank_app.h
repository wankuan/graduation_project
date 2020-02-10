#ifndef __TANK_H__
#define __TANK_H__
#include "lib_pub.h"
#include "data_packet.h"
#define INVALID_POINTER(pointer) (pointer==NULL)

#define APP_FIFO_SIZE 256
#define APP_BUF_MAX_SIZE 2048
typedef enum{
    FAIL = 0,
    SUCCESS = 1
}APP_STATUS;

// typedef struct __msg_buf_t{
//     msg_type_t data_type;
//     uint32_t length;
//     uint8_t *buf_p;
// }msg_buf_t;
typedef APP_STATUS app_status_t;
typedef int system_id_t;
typedef int app_id_t;


typedef struct msg_FIFO{
    uint32_t pop_p;
    uint32_t push_p;
    app_packet_t* buf[APP_FIFO_SIZE];
}msg_FIFO_t;

typedef struct __tank_app_t{
    system_id_t system_ID;
    app_id_t app_ID;
    uint8_t preempt_priority;
    uint8_t sub_priority;
    msg_FIFO_t read_buf;
    msg_FIFO_t send_buf;
}tank_app_t;

int tank_app_init(tank_app_t *tank_app);
int tank_app_deinit(tank_app_t *tank_app);

app_status_t tank_app_send(tank_app_t *tank_app, const uint8_t* buf, uint32_t size);
app_status_t tank_app_receive(tank_app_t *tank_app, const uint8_t* buf, uint32_t size);


app_status_t tank_app_link_refresh(tank_app_t *tank_app);

#endif


