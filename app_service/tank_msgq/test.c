#include "tank_pub.h"
#include "tank_msgq.h"

int main(int argc, char *argv[])
{
    tank_msgq_t msgq;
    tank_msgq_creat(&msgq, 10);
    strncpy(msgq.buf[0], "huang", 10);
    strncpy(msgq.buf[1], "huang1", 10);
    strncpy(msgq.buf[2], "huang2", 10);
    printf("%p\n",&msgq.buf[0]);
    printf("%p\n",&msgq.buf[1]);
    printf("%p\n",msgq.buf[2]);
    printf("%s\n",msgq.buf[0]);
    printf("%s\n",msgq.buf[1]);
    printf("%s\n",msgq.buf[2]);
    return 0;
}


