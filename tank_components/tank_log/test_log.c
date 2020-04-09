#include "tank_log_api.h"
#include <unistd.h>
#include <stdarg.h>

tank_log_t mylog;
#define log_debug(fmt,...) tank_log_output(&mylog,"LOG_DEMO", LEVEL_DEBUG,fmt,##__VA_ARGS__)
#define log_info(fmt,...)  tank_log_output(&mylog,"LOG_DEMO", LEVEL_INFO,fmt,##__VA_ARGS__)
#define log_error(fmt,...) tank_log_output(&mylog,"LOG_DEMO", LEVEL_ERROR,fmt,##__VA_ARGS__)
#define log_warn(fmt,...)  tank_log_output(&mylog,"LOG_DEMO", LEVEL_WARNING,fmt,##__VA_ARGS__)


int main(int argv, char*args[])
{
    tank_log_init(&mylog, "logfile.log",2048,
                LOG_INFO_TIME|LOG_INFO_OUTAPP|LOG_INFO_FUNC|LOG_INFO_FILE|LOG_INFO_LEVEL,
                PORT_SHELL|PORT_FILE
                );
    log_debug("This is a debug log\n");
    log_error("This is a error log\n");
    log_info("This is a info log\n");
    log_warn("This is a warning log\n");
    tank_log_destructor(&mylog);
    return 0;
}
