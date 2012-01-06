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

#define LOCATION_INIT(a,b,c,d) { \
 .latitude = a, \
 .longitude = b, \
 .altitude = c, \
 .accuracy = d, \
 .reserved = {0}, \
}

#endif //LOCATION_H
