#ifndef STEP_INFO_H
#define STEP_INFO_H

#include "types.h"

#define MAX_STEP_ENTRIES 50

typedef struct _StepInfo
{
    uint16  sampleFrequency; //2 bytes (How many secs of batt life left)
    uint8  updateFrequency;  //1 byte (Number of minutes per Upload)
    uint8  numberOfEntries; //1 byte (# of step measurements)
    //uint16 *steps;         //numberOfEntries*2 bytes (actual step data)
    uint16 steps[MAX_STEP_ENTRIES];
}
StepInfo;

#endif //STEP_INFO_H
