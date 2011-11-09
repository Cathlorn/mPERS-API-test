#ifndef LOCATION_H
#define LOCATION_H

#include "types.h"

typedef struct _Location
{
    uint32 latitude;
    uint32 longitude;
    uint32 altitude;
    uint16 accuracy;
    uint8  reserved[2];
} Location;

#endif //LOCATION_H
