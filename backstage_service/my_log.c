#include "my_log.h"
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>


void get_current_time(uint8_t *p_timer)
{
    struct timeval tv;
    struct tm  *p;
    gettimeofday(&tv, NULL);
    p = localtime(&tv.tv_sec);
    sprintf((char*)p_timer,"%04d-%02d-%02d %02d:%02d:%02d.%03lu", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec/1000);
}
void write_buffer(const char *fmt, ...)
{
    FILE *file_fp;
    char fmt_str[1024];
    char time_buf[30];
    get_current_time(time_buf);
    snprintf(fmt_str, 1024, "[%s] ",time_buf);


    va_list ap;
    va_start(ap, fmt);
    vsnprintf(&fmt_str[strlen(fmt_str)], sizeof(fmt_str), fmt, ap);
    va_end(ap);

    printf("%s",fmt_str);
    file_fp = fopen("./test.log", "a");

    // 写入到日志文件中
    if (file_fp != NULL)
    {
        fprintf(file_fp, "%s", fmt_str);
        //fflush(file_fp);
        fclose(file_fp);
    }
}
