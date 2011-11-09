#ifndef ACTIVITY_INFO_H
#define ACTIVITY_INFO_H

#include "types.h"

#define MAX_ACTIVITY_ENTRIES 50

typedef struct _ActivityInfo
{
    uint16  sampleFrequency; //2 bytes (How many secs of batt life left)
    uint8   updateFrequency;  //1 byte (Number of minutes per Upload)
    uint8  numberOfEntries; //1 byte (# of activity entries)
    //uint16 *activities;    //numberOfEntries*2 bytes (actual activity data in Calories)

    uint16 activities[MAX_ACTIVITY_ENTRIES];
}
ActivityInfo;

#endif //ACTIVITY_INFO_H
