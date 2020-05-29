// #include "external_process.h"
#include "tank_delay.h"
#include "uart.h"
// external_process_info_t ex_pro_demo;

#include "tank_log_api.h"
#define FILE_NAME "external_process_service"

const char *send_str = "test_write_into_uart\n";


tank_status_t uart_recv_handler(void *buf, uint16_t *len)
{
    char *recv_str = (char*)buf;
    printf("recv data is ======\n");
    printf("%s\n", recv_str);
    printf("==============\n");
    return TANK_SUCCESS;
}


int main(int argc, char *argv[])
{
    tank_log_init(&mylog, "external_process",2048, LEVEL_DEBUG,
            LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
            PORT_FILE|PORT_SHELL
            );
    log_info("========logger start===========\n");
    // external_service_init(&ex_pro_demo);
    hal_uart_init(uart_recv_handler);
    while(1){
        // hal_uart_send(send_str, strlen(send_str));
        // // log_info("============entry running!============\n");
        sleep_ms(1000);
        // hal_uart_read();
    }
    return 0;
}
