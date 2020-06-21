#include "tank_delay.h"
#include "uart.h"
#include "tank_log_api.h"
#define FILE_NAME "UART_TEST"

#define UART_TEST_FRAME 12
uint8_t send_str[] = {0xFE, 0X00, 0X01, 0X02, 0X03, 0X04, 0X05, 0X06, 0X07, 0X08, 0X09, 0XEF};



tank_status_t uart_recv_handler(void *buf, uint16_t *len)
{
    log_info("recv data is ======, len is %d\n", *len);
    for(uint16_t i=0;i<*len;++i){
        printf("0x%x ", *((uint8_t*)buf+i));
    }
    printf("\n");
    return TANK_SUCCESS;
}


int main(int argc, char *argv[])
{
    if(!strncmp(argv[1], "send", 1024)){
        tank_log_init(&mylog, "uart_send",2048, LEVEL_INFO,
            LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
            PORT_FILE|PORT_SHELL
            );
        log_info("========logger start===========\n");
        hal_uart_init("/dev/ttyUSB0", uart_recv_handler);
        while(1){
            hal_uart_send(send_str, UART_TEST_FRAME);
            printf("sleep\n");
            sleep_ms(1000);
        }
    }else if(!strncmp(argv[1], "recv", 1024)){
        tank_log_init(&mylog, "uart_recv",2048, LEVEL_INFO,
            LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
            PORT_FILE|PORT_SHELL
            );
        log_info("========logger start===========\n");
        hal_uart_init("/dev/ttyUSB1", uart_recv_handler);
        while(1){
            // hal_uart_send(send_str, UART_TEST_FRAME);
            // printf("sleep\n");
            sleep_ms(1000);
        }
    }
    return 0;
}
