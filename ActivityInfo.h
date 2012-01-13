#ifndef ACTIVITY_INFO_H
#define ACTIVITY_INFO_H

#include "types.h"

#define MAX_ACTIVITY_ENTRIES 50

typedef struct _ActivityInfo
{
    uint16  sampleRate; //2 bytes (How many secs of batt life left)
    uint16  numberOfEntries; //1 byte (# of activity entries)
    uint16 activities[MAX_ACTIVITY_ENTRIES];
}
ActivityInfo;

#define ACTIVITY_INFO_INIT() { \
 .sampleRate  = 0, \
 .numberOfEntries = 0, \
 .activities = {0}, \
}

#endif //ACTIVITY_INFO_H
