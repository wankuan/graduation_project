#include "external_process.h"
#include "tank_delay.h"
#include "uart.h"
external_process_info_t ex_pro_demo;

#include "tank_log_api.h"
#define FILE_NAME "external_process_service"

int main(int argc, char *argv[])
{
    tank_log_init(&mylog, "external_process",2048, LEVEL_DEBUG,
            LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_LEVEL,
            PORT_FILE|PORT_SHELL
            );
    log_info("========logger start===========\n");
    external_service_init(&ex_pro_demo);
    while(1){
        log_info("============entry running!============\n");
        sleep_ms(1000);
    }
    return 0;
}
