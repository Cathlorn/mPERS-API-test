#ifndef HALO_MESSAGE_H
#define HALO_MESSAGE_H

#include "types.h"

//Generic Message structure. All Messages will contain these and are essential for proper operation
typedef struct _HaloMessage
{
    uint8 formatVersion;
    uint8 reserved;
    uint16 commandType;
    uint32 deviceId;          //Needs to be at least this size whatever we use
    uint32 time;              //Needs to be at least this size whatever we use
} HaloMessage;

#define HALO_MESSAGE_INIT(a,b,c,d) { \
 .formatVersion = a, \
 .reserved = 0, \
 .commandType = b, \
 .deviceId = c, \
 .time = d, \
}

#endif //HALO_MESSAGE_H
