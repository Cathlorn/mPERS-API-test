#ifndef HALO_MESSAGE_H
#define HALO_MESSAGE_H

#include "types.h"

//Generic Message structure. All Messages will contain these and are essential for proper operation
typedef struct
{
//    uint16 formatVersion : 4; //Allows a message type to change formats in the future.
    //uint16 commandType : 12;  //4096 diff msgs using enum MsgTypes. An extension could be made.
    //uint16 formatVersion : 4; //Allows a message type to change formats in the future.
    //uint8  reserved[2];
    uint8 formatVersion;
    uint8 reserved;
    uint16 commandType;
    uint32 deviceId;          //Needs to be at least this size whatever we use
    uint32 time;              //Needs to be at least this size whatever we use
} HaloMessage;

#define HALO_MESSAGE_INIT(a,b,c,d) { \
 .formatVersion = b, \
 .reserved = 0, \
 .commandType = a, \
 .deviceId = c, \
 .time = d, \
}

#endif //HALO_MESSAGE_H
