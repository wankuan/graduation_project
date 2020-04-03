#include <pthread.h>
#include "tank_pub.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define tank_mm "/tank_mm"


int main(int argc, char *argv[])
{
    const char *name = tank_mm;
    int fd = shm_open(name, O_RDWR, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    /* 更改文件长度，要比共享内存写入区域大 */
    uint16_t map_size = 512;
    printf("map_size:%d\n", map_size);

    char *buf = (char*)mmap(NULL, map_size, PROT_READ, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    printf("mmap address:%p\n", buf);

    // msync(&buf, map_size, MS_ASYNC);
    printf("read:%s", buf+8);
    munmap(buf, map_size);
    close(fd);
    return 0;
}