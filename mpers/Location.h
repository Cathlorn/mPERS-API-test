#ifndef LOCATION_H
#define LOCATION_H

#include "General/types.h"

typedef struct
{
    uint32 latitude;
    uint32 longitude;
    uint32 elevation;
    uint16 accuracy;
    uint16 deltaElevation;
} Location;

#define LOCATION_INIT(a,b,c,d) { \
 .latitude  = a, \
 .longitude = b, \
 .elevation = c, \
 .accuracy  = d, \
 .deltaElevation = 0, \
}

#endif //LOCATION_H
