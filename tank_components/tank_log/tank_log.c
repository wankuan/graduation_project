#include "tank_log.h"
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>

static log_status_t get_log_level_str(uint32_t level, char *level_str);
static log_status_t get_current_time_str(uint8_t *p_timer);
static log_status_t write_file(FILE *fp, char *buffer);
static log_status_t write_shell(char *buffer);
static log_status_t write_uart(char *buffer);

static const char* log_level_str[] = {
        "DEBUG",
        "INFO",
        "NOTE",
        "WARNING",
        "ERROR",
        "CRITICAL",
        "ALERT",
        "EMERGENCY"
};

static log_status_t get_log_level_str(uint32_t level, char *level_str)
{
    if(level_str==NULL_PTR)
        return LOG_FAIL;
    strncpy(level_str, log_level_str[level],LOG_INFO_LEVEL_WIDTH);
    return LOG_SUCCESS;
}

static log_status_t get_current_time_str(uint8_t *p_timer)
{
    struct timeval tv;
    struct tm  *p;
    if(p_timer==NULL_PTR)
        return LOG_FAIL;
    gettimeofday(&tv, NULL);
    p = localtime(&tv.tv_sec);
    sprintf((char*)p_timer,"%04d-%02d-%02d %02d:%02d:%02d.%03lu", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec/1000);
    return LOG_SUCCESS;
}


log_status_t tank_log_constructor(tank_log_t *log_handler)
{
    FILE *pfile = NULL;
    log_file_t *file_handler = NULL;

    file_handler = &log_handler->file_handler;
    pfile = fopen(file_handler->name, "a+");
    if(pfile==NULL_PTR){
        printf("[ERROR]fopen error, invalid pointor\n");
        return LOG_FAIL;
    }


    file_handler->FILE_IO = pfile;
    return LOG_SUCCESS;
}

log_status_t tank_log_destructor(tank_log_t *log_handler)
{
    log_handler->file_handler.size = 0;
    log_handler->file_handler.name[0] = '\0';
    fclose(log_handler->file_handler.FILE_IO);
    return LOG_SUCCESS;
}


// [时间][输出app][文件信息][函数信息][日志等级]:日志
log_status_t tank_log_write(tank_log_t *log_handler, const char *app, const char *filename, const char *fun, log_level_t level, char *fmt, ...)
{
    uint32_t log_info;
    uint32_t write_len = 0;
    char log_buffer[LOG_SINGLE_WIDTH];
    if(level < log_handler->info_handler.level){
        return TANK_SUCCESS;
    }
    log_info = log_handler->info_handler.type&LOG_INFO_MASK;
    if(log_info&LOG_INFO_TIME){
        uint8_t time_buffer[LOG_INFO_TIME_WIDTH];
        get_current_time_str(time_buffer);
        snprintf(&log_buffer[write_len], LOG_INFO_TIME_WIDTH, "[%s]",time_buffer);
        write_len = strlen(log_buffer);
    }
    if(log_info&LOG_INFO_LEVEL){
        char level_str_buffer[LOG_INFO_LEVEL_WIDTH];
        get_log_level_str(level, level_str_buffer);
        snprintf(&log_buffer[write_len], LOG_INFO_LEVEL_WIDTH, "[%s]",level_str_buffer);
        write_len = strlen(log_buffer);
    }
    if(log_info&LOG_INFO_OUTAPP){
        snprintf(&log_buffer[write_len], LOG_INFO_APP_WIDTH, "[%s]",app);
        write_len = strlen(log_buffer);
    }
    if(log_info&LOG_INFO_FILE){
        snprintf(&log_buffer[write_len], LOG_INFO_FILE_WIDTH, "[%s]",filename);
        write_len = strlen(log_buffer);
    }
    if(log_info&LOG_INFO_FUNC){
        snprintf(&log_buffer[write_len], LOG_INFO_FUN_WIDTH, "[%s]",fun);
        write_len = strlen(log_buffer);
    }


    snprintf(&log_buffer[write_len], 256, ":");
    write_len = strlen(log_buffer);

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(&log_buffer[write_len], LOG_SINGLE_WIDTH, fmt, ap);
    va_end(ap);

    log_out_port port = log_handler->info_handler.port;
    if(port&PORT_SHELL){
        write_shell(log_buffer);
    }
    if(port&PORT_FILE){
        write_file(log_handler->file_handler.FILE_IO, log_buffer);
    }
    if(port&PORT_SHELL){
        write_uart(log_buffer);
    }
    return TANK_SUCCESS;
}

static log_status_t write_file(FILE *fp, char *buffer)
{
    if (fp == NULL_PTR){
        printf("[ERROR]fp NULL_PTR\n");
        return LOG_FAIL;
    }
    fprintf(fp, "%s", buffer);
    fflush(fp);
    return LOG_SUCCESS;
}

static log_status_t write_shell(char *buffer)
{
    printf("%s",buffer);
    return LOG_SUCCESS;
}

static log_status_t write_uart(char *buffer)
{
    // TODO
    //printf("no uart\n");
    return LOG_SUCCESS;
}