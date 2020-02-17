#ifndef __OSAL_PUB_H__
#define __OSAL_PUB_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h>


#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

key_t osal_get_keyid(void);


#endif