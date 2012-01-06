#ifndef STEP_INFO_H
#define STEP_INFO_H

#include "types.h"

#define MAX_STEP_ENTRIES 50

typedef struct _StepInfo
{
    uint16  sampleFrequency; //2 bytes (How many secs of batt life left)
    uint8  updateFrequency;  //1 byte (Number of minutes per Upload)
    uint8  numberOfEntries;  //1 byte (# of step measurements)
    uint16 steps[MAX_STEP_ENTRIES];
}
StepInfo;

#define STEP_INFO_INIT() { \
 .sampleFrequency  = 0, \
 .updateFrequency = 0, \
 .numberOfEntries = 0, \
 .steps = {0}, \
}

//Multiple Index assignment
//.steps = {[0 ... MAX_STEP_ENTRIES-1] = 0},

#endif //STEP_INFO_H
