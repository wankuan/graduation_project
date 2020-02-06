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
    uint8_t *msg_send = NULL;
    if(INVALID_POINTER(tank_app)||INVALID_POINTER(buf)){
        goto fail;
    }
    printf("--------send messgae--------\n");
    printf("%s\n",buf);

    msg_send = (uint8_t*)malloc(size);
    if(INVALID_POINTER(msg_send)){
        goto fail;
    }
    memcpy(msg_send, buf, size);
    uint32_t *write_p = &(tank_app->send_buf.write_p);

    if(*write_p >= tank_app->send_buf.length){
        *write_p = 0;
    }
    tank_app->send_buf.buf[*write_p].data_type = BYTE;
    tank_app->send_buf.buf[*write_p].length = size;
    tank_app->send_buf.buf[*write_p].buf_p = msg_send;
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