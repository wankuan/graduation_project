#include "osal_pub.h"

key_t osal_get_keyid(void)
{
    key_t key_id;
    key_id = ftok("/",0x00);
    printf("key id is %d\n",key_id);
    return key_id;
}