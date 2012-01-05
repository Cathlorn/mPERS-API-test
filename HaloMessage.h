#ifndef HALO_MESSAGE_H
#define HALO_MESSAGE_H

#include "types.h"

//Generic Message structure. All Messages will contain these and are essential for proper operation
typedef struct _HaloMessage
{
//    uint16 formatVersion : 4; //Allows a message type to change formats in the future.
    uint16 commandType : 12;  //4096 diff msgs using enum MsgTypes. An extension could be made.
    uint16 formatVersion : 4; //Allows a message type to change formats in the future.
    uint8  reserved[2];
    uint32 deviceId;          //Needs to be at least this size whatever we use
    uint32 time;              //Needs to be at least this size whatever we use
} HaloMessage;

#endif //HALO_MESSAGE_H
