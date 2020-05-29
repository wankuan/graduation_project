#include "tank_delay.h"
#include "uart.h"

#include "tank_log_api.h"

const char *send_str = "1234723928dsvsdvsdsdvdsv";




tank_status_t uart_recv_handler(void *buf, uint16_t *len)
{
    char *recv_str = (char*)buf;
    if(*len != strlen(send_str)){
        printf("=======[ERROR]recv error, current length is %d\n", *len);
    }
    printf("recv data is ======, len is %d\n", *len);
    printf("%s\n", recv_str);
    return TANK_SUCCESS;
}


int main(int argc, char *argv[])
{
    if(!strncmp(argv[1], "send", 1024)){
        hal_uart_init("/dev/ttyUSB0", uart_recv_handler);
        while(1){
            hal_uart_send(send_str, strlen(send_str));
            sleep_ms(100);
        }
    }else if(!strncmp(argv[1], "recv", 1024)){
        hal_uart_init("/dev/ttyUSB1", uart_recv_handler);
        while(1){
            sleep_ms(1000);
        }
    }
    return 0;
}
