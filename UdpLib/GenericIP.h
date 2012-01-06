#ifndef GENERIC_IP_H
#define GENERIC_IP_H

#include "../types.h"

//Making IPv4-specific for now, can add support for IPv6 later
typedef struct
{
    uint32  address;
    uint16  port;
    uint8   reserved[2]; //padding to make 4-byte-aligned
}
GenericIP;

#define GENERIC_IP_INIT() { \
 .address     = 0, \
 .port        = 0, \
 .reserved    = {0}, \
}

#endif //GENERIC_IP_H
