#include "tank_map.h"

volatile uint32_t g_shm_base = 0u;

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
    g_shm_base = (uint32_t)buf;
    printf("[MAP]shm_base:0x%x  addr:%p\n", g_shm_base, &g_shm_base);
    close(fd);
    return TANK_SUCCESS;
}

tank_status_t get_service_msgq_addr(tank_msgq_t **addr)
{
    *addr = (tank_msgq_t *)(*(uint32_t *)MSGQ_MAP_ADDR + g_shm_base);
    printf("[MAP]service msgq addr:%p\n", *addr);
    return TANK_SUCCESS;
}