#include "tank_app.h"

int tank_app_init(tank_app_t *tank_app)
{
    tank_app->send_buf.length = 256;
    tank_app->send_buf.write_p = 0;

    tank_app->read_buf.length = 256;
    tank_app->read_buf.write_p = 0;
    return 0;
}
int tank_app_deinit(tank_app_t *tank_app)
{
    return 0;
}

int tank_app_send(tank_app_t *tank_app, const uint8_t* buf, uint32_t size)
{
    app_packet_t *msg_send = NULL;
    if(INVALID_POINTER(tank_app)||INVALID_POINTER(buf)){
        goto fail;
    }

    msg_send = (app_packet_t*)malloc(sizeof(app_packet_t)+size);
    if(INVALID_POINTER(msg_send)){
        goto fail;
    }
    msg_send->len = size;
    msg_send->ID_system_src = SYSTEM_A;
    msg_send->ID_system_dst = SYSTEM_B;
    memcpy(msg_send->data, buf, size);
    uint32_t *write_p = &(tank_app->send_buf.write_p);
    if(*write_p >= tank_app->send_buf.length){
        *write_p = 0;
    }
    tank_app->send_buf.buf[*write_p] = msg_send;
    printf("messgae:%s\n",msg_send->data);
    printf("size:%d\n",msg_send->len);
    printf("from ID:0x%x to ID:0x%x\n",msg_send->ID_system_src, msg_send->ID_system_dst);
    (*write_p)++;
    return 0;
fail:
    return 1;
}
int tank_app_receive(tank_app_t *tank_app, const uint8_t* buf, uint32_t size)
{
    if(INVALID_POINTER(tank_app)||INVALID_POINTER(buf)){
        return 1;
    }
    printf("--------receive messgae--------\n");
    printf("%s\n",buf);
    return 0;
}