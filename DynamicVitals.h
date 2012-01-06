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

#define DYNAMIC_VITALS_MESSAGE_INIT() { \
                                .baseMessage = HALO_MESSAGE_INIT(ALL_DATA_DYNAMIC, 0, 0, 0), \
                                .battInfo = BATTERY_INFO_INIT(), \
                                .currentLocation = LOCATION_INIT(0, 0, 0, 0), \
                                .stepData = STEP_INFO_INIT(), \
                                .activityData = ACTIVITY_INFO_INIT(), \
                          }

//Helper Function to help with dynamic changes

//Populates the structure correctly. Needed since partial xmission is done to save space.
void pack_DynamicVitals(const DynamicVitals *msg, void *dstData);
void unpack_DynamicVitals(const void *srcData, DynamicVitals *msg);

#endif //DYNAMIC_VITALS_H
