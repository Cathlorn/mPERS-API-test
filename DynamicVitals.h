#ifndef DYNAMIC_VITALS_H
#define DYNAMIC_VITALS_H

#include "types.h"
#include "ActivityInfo.h"
#include "BatteryInfo.h"
#include "HaloMessage.h"
#include "Location.h"
#include "StepInfo.h"

typedef struct _DynamicVitals
{
    HaloMessage baseMessage;
    BatteryInfo battInfo;     //4 bytes for now
    Location currentLocation; //14 bytes for now
    StepInfo stepData;        //4 + numberOfEntries*2 bytes; numberOfEntries=10: 24 bytes
    ActivityInfo activityData;//4 + numberOfEntries*2 bytes; numberOfEntries=10: 24 bytes
}
DynamicVitals;

//Helper Function to help with dynamic changes

//Populates the structure correctly. Needed since partial xmission is done to save space.
void pack_DynamicVitals(DynamicVitals *msg, void *dstData);
void unpack_DynamicVitals(void *srcData, DynamicVitals *msg);

#endif //DYNAMIC_VITALS_H
