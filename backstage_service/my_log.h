#ifndef __MY_LOG_H__
#define __MY_LOG_H__

#include "lib_pub.h"


// #define LOG_RECORD(logLev, fmt, ...)
//     printf(fmt, __VA_ARGS__)


void get_current_time(uint8_t *p_timer);
void write_buffer(const char *fmt, ...);

#define LOG_S(fmt, ...) write_buffer(fmt,__VA_ARGS__)

// #define LOG_S(fmt, ...)\
//     do{\
//         char fmt_str[1024];\
//         char time_buf[30];\
//         get_current_time(time_buf);\
//         snprintf(fmt_str,1024,"%s %s",time_buf,fmt,__VA_ARGS__);\
//         printf("%s\n",fmt_str);\
//     }while(0)



#endif