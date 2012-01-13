#ifndef SIGNAL_STRENGTH_H
#define SIGNAL_STRENGTH_H

#include "types.h"

typedef struct
{
    uint32 cellularStrength; //dBm
} SignalStrength;

#define SIGNAL_STRENGTH_INIT() { \
 .cellularStrength = 0, \
}

#endif //SIGNAL_STRENGTH_H
