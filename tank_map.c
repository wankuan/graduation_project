#include "tank_map.h"

volatile uint32_t shm_base_s = 0u;

tank_status_t get_service_base_addr(void)
{
    const char *name = TANK_PUB_NAME;
    int fd = shm_open(name, O_RDWR, S_IRUSR|S_IWUSR);
    if(fd < 0){
        perror("creat ERROR");
        exit(1);
    }
    void *buf = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (!buf) {
        printf("mmap failed\n");
        close(fd);
        exit(1);
    }
    shm_base_s = (uint32_t)buf;
    printf("[MAP]shm_base:0x%x  addr:%p\n", shm_base_s, &shm_base_s);
    close(fd);
    return TANK_SUCCESS;
}

tank_status_t get_service_msgq_addr(tank_msgq_t **addr)
{
    *addr = (tank_msgq_t *)(*(uint32_t *)MSGQ_MAP_ADDR + shm_base_s);
    printf("[MAP]service msgq addr:%p\n", *addr);
    return TANK_SUCCESS;
}