#ifndef __TANK_ID_H__
#define __TANK_ID_H__

#define INNER_SERVICE_ID 0x00
#define PACKAGE_MAX_SIZE (1024)

#define PACKAGE_MASK (0xFFFF>>5)
#define PACKAGE_SLICE(x) (((x + 31)>>5) & (PACKAGE_MASK))

typedef enum{
    PACK_32   = (0x01 << 0),
    PACK_64   = (0x01 << 1),
    PACK_128  = (0x01 << 2),
    PACK_256  = (0x01 << 3),
    PACK_512  = (0x01 << 4),
    PACK_1024 = (0x01 << 5),
}package_size_type_t;

enum{
    APP_A = 0x00,
    APP_B,
    APP_C,
    APP_D,
}tank_id_table_t;



#endif