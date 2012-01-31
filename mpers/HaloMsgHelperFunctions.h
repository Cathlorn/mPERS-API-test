#ifndef HALO_MSG_HELPER_FUNCTIONS_H
#define HALO_MSG_HELPER_FUNCTIONS_H

#include "HaloMessage.h"

void init_base_message(uint16 commandType, uint8 formatVersion,
                       uint32 timeStamp, uint32 deviceId,
                       HaloMessage *message);

int getMsgLength(const HaloMessage *msg);

#endif //HALO_MSG_HELPER_FUNCTIONS_H
