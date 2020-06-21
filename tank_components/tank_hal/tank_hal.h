#ifndef __TANK_HAL_H__
#define __TANK_HAL_H__

#include "tank_pub.h"

typedef enum{
    IIC_LINK,
    SOCKET_LINK,
    SPI_LINK,
    UART_LINK
}tank_hal_link_types_t;


tank_status_t tank_hal_send(tank_hal_link_types_t link_type, void *buf, uint32_t size);

tank_status_t tank_hal_recv(tank_hal_link_types_t *link_type, void *buf, uint32_t *size);
#endif