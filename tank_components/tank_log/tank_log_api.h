#ifndef __TANK_LOG_API_H__
#define __TANK_LOG_API_H__

#include "tank_pub.h"
#include "tank_log.h"
extern tank_log_t mylog;

tank_status_t tank_log_init(tank_log_t *log_handler, char *filename, uint32_t size, log_level_t level, log_info_type info_type, log_out_port port);
tank_status_t tank_log_denint(tank_log_t *log_handler);
#ifdef TANK_DEBUG
// #define FILE_NAME "tank_mm"

#define log_debug(fmt,...) tank_log_output(&mylog, FILE_NAME, LEVEL_DEBUG,fmt,##__VA_ARGS__)
#define log_info(fmt,...)  tank_log_output(&mylog, FILE_NAME, LEVEL_INFO,fmt,##__VA_ARGS__)
#define log_error(fmt,...) tank_log_output(&mylog, FILE_NAME, LEVEL_ERROR,fmt,##__VA_ARGS__)
#define log_warn(fmt,...)  tank_log_output(&mylog, FILE_NAME, LEVEL_WARNING,fmt,##__VA_ARGS__)
#else
    #define log_debug(fmt,...)
    #define log_info(fmt,...)
    #define log_error(fmt,...)
    #define log_warn(fmt,...)
#endif


#define tank_log_output(log_handler,app,level,fmt,...) \
    do{\
        char file_str[256];\
        char function_str[32];\
        char app_str[32];\
        snprintf(app_str, 32, "%s",app);\
        snprintf(file_str, 256, "%s",__FILE__);\
        snprintf(function_str, 32, "%s",__func__);\
        tank_log_write(log_handler,app, __FILE__, __func__,level,fmt,##__VA_ARGS__);\
    }while(0)


#endif