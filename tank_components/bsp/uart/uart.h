#ifndef __UART_H__
#define __UART_H__

#include "tank_pub.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> //文件控制定义
#include <termios.h>//终端控制定义
#include <errno.h>


typedef tank_status_t (*uart_recv_cb_t)(void *buf, uint16_t *len);

tank_status_t hal_uart_init(const char *device_name, uart_recv_cb_t cb);

int hal_uart_send(uint8_t *data, int datalen);
tank_status_t hal_uart_read(void);


#endif