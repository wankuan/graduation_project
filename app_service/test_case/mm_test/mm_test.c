#include <pthread.h>
#include "tank_pub.h"
#include "tank_msgq.h"
#include "tank_mm.h"
#include "my_sem.h"
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define tank_mm "/tank_mm"

tank_mm_t my_mm;

int main(int argc, char *argv[])
{
    const char *name = tank_mm;
    int fd = shm_open(name, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    /* 更改文件长度，要比共享内存写入区域大 */
    uint16_t map_size = 256;
    printf("map_size:%d\n", map_size);
    ftruncate(fd, map_size);

    char *buf = (char*)mmap(NULL, map_size, PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    printf("mmap address:%p\n", buf);

    tank_mm_register(&my_mm, (uint32_t)(buf+20), map_size, "huang");

    void * p = tank_mm_alloc(&my_mm, 16);
    printf("addr:%p, end:%p,total_size:%d\n", my_mm.heap.addr, my_mm.heap.pxEnd, my_mm.heap.total_size);
    printf("xFreeBytesRemaining:%d, xMinimumEverFreeBytesRemaining:%d,start:%p, size:%d\n", my_mm.heap.xFreeBytesRemaining,
     my_mm.heap.xMinimumEverFreeBytesRemaining, &my_mm.heap.xStart.pxNextFreeBlock, my_mm.heap.xStart.xBlockSize);
    printf("addr:%p\n", &my_mm);

    BlockLink_t *a;
    a = (BlockLink_t *)(p - 8);
    printf("%p,%x,size:%x,%p\n", a, a->pxNextFreeBlock, a->xBlockSize, &a->xBlockSize);
    for(int i=0;i<8;i++)
    {
        printf("%x ",*(uint8_t*)((void*)(a)+i));
    }
    printf("\nsize:%x\n", *(uint32_t*)((void*)a+4));
    if(p == NULL){
        printf("allocate error\n");
    }else{
        printf("allocate success addr:%p\n", p);
    }
    memcpy(p, "test mm", strlen("test mm"));
    tank_mm_free(&my_mm, p);
    // snprintf(buf, 1024, "This is a share memory!");
    // printf("message:%s\n", buf);

    // munmap(buf, map_size);
    // close(fd);
    return 0;
}