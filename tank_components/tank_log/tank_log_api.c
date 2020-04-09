#include "tank_log_api.h"
#include <sys/time.h>
#include <time.h>
tank_log_t mylog;

static log_status_t get_current_time_str(char *p_timer)
{
    struct timeval tv;
    struct tm  *p;
    if(p_timer==NULL_PTR)
        return LOG_FAIL;
    gettimeofday(&tv, NULL);
    p = localtime(&tv.tv_sec);
    sprintf((char*)p_timer,"%04d_%02d_%02d___%02d_%02d", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min);
    return LOG_SUCCESS;
}



tank_status_t tank_log_init(tank_log_t *log_handler, char *filename, uint32_t size, log_info_type info_type, log_out_port port)
{
    log_status_t status;
    log_handler->file_handler.size = size;
    log_handler->info_handler.type = info_type;
    log_handler->info_handler.port = port;

    char time[32];
    get_current_time_str(time);
    snprintf(log_handler->file_handler.name, 256, "%s_%s.log",filename, time);
    status = tank_log_constructor(log_handler);
    if (status != LOG_SUCCESS){
        return TANK_FAIL;
    }
    return TANK_SUCCESS;
}
tank_status_t tank_log_denint(tank_log_t *log_handler)
{
    log_status_t status;
    status = tank_log_destructor(log_handler);
    if (status == LOG_SUCCESS){
        return TANK_SUCCESS;
    }else{
        return TANK_FAIL;
    }
}

