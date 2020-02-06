#ifndef __TANK_H__
#define __TANK_H__
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define INVALID_POINTER(pointer) (pointer==NULL)

typedef enum{
BYTE = 0,
HALFWORD = 1,
WORD = 2
}msg_type_t;

typedef struct __msg_buf_t{
    msg_type_t data_type;
    uint32_t length;
    uint8_t *buf_p;
}msg_buf_t;

struct msg_FIFO{
    uint32_t length;
    uint32_t write_p;
    msg_buf_t buf[256];
};

typedef struct __tank_app_t{
    uint8_t system_ID;
    uint8_t app_ID;
    uint8_t preempt_priority;
    uint8_t sub_priority;
    struct msg_FIFO read_buf;
    struct msg_FIFO send_buf;
}tank_app_t;

int tank_app_init(tank_app_t *tank_app);
int tank_app_deinit(tank_app_t *tank_app);

int tank_app_send(tank_app_t *tank_app, const uint8_t* buf, uint32_t size);
int tank_app_receive(tank_app_t *tank_app, const uint8_t* buf, uint32_t size);


#endif


