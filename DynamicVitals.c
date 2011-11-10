#include <string.h>

#include "DynamicVitals.h"

//Populates the structure correctly. Needed since partial xmission is done to save space.
void pack_DynamicVitals(const DynamicVitals *msg, void *dstData)
{
    StepInfo *stepInfoPtr;
    ActivityInfo *activityInfoPtr;
    uint8 *dataPtr;

    dataPtr = (uint8 *) dstData;

    memcpy(dataPtr, &msg->baseMessage, sizeof(HaloMessage));
    dataPtr += sizeof(HaloMessage);

    memcpy(dataPtr, &msg->battInfo, sizeof(BatteryInfo));
    dataPtr += sizeof(BatteryInfo);

    memcpy(dataPtr, &msg->currentLocation, sizeof(Location));
    dataPtr += sizeof(Location);

    stepInfoPtr = (StepInfo *) dataPtr;
    stepInfoPtr->sampleFrequency = msg->stepData.sampleFrequency;
    stepInfoPtr->updateFrequency = msg->stepData.updateFrequency;
    stepInfoPtr->numberOfEntries = msg->stepData.numberOfEntries;
    memcpy(&stepInfoPtr->steps[0], &msg->stepData.steps[0], sizeof(uint16)*msg->stepData.numberOfEntries);

    dataPtr = (uint8 *) &stepInfoPtr->steps[0];
    dataPtr += (sizeof(uint16)*stepInfoPtr->numberOfEntries);
    activityInfoPtr = (ActivityInfo *) dataPtr;
    activityInfoPtr->sampleFrequency = msg->activityData.sampleFrequency;
    activityInfoPtr->updateFrequency = msg->activityData.updateFrequency;
    activityInfoPtr->numberOfEntries = msg->activityData.numberOfEntries;
    memcpy(&activityInfoPtr->activities[0], &msg->activityData.activities[0], sizeof(uint16)*msg->activityData.numberOfEntries);
}

void unpack_DynamicVitals(const void *srcData, DynamicVitals *msg)
{
    DynamicVitals *srcMsg = (DynamicVitals *) srcData;
    StepInfo *stepInfoPtr;
    ActivityInfo *activityInfoPtr;
    uint8 *dataPtr;

    memcpy(&msg->baseMessage, &srcMsg->baseMessage, sizeof(HaloMessage));
    memcpy(&msg->battInfo, &srcMsg->battInfo, sizeof(BatteryInfo));
    memcpy(&msg->currentLocation, &srcMsg->currentLocation, sizeof(Location));

    stepInfoPtr = &srcMsg->stepData;
    msg->stepData.sampleFrequency = stepInfoPtr->sampleFrequency;
    msg->stepData.updateFrequency = stepInfoPtr->updateFrequency;
    msg->stepData.numberOfEntries = stepInfoPtr->numberOfEntries;
    memcpy(&msg->stepData.steps[0], &stepInfoPtr->steps[0], sizeof(uint16)*stepInfoPtr->numberOfEntries);

    dataPtr = (uint8 *) &stepInfoPtr->steps[0];
    dataPtr += (sizeof(uint16)*stepInfoPtr->numberOfEntries);
    activityInfoPtr = (ActivityInfo *) dataPtr;
    msg->activityData.sampleFrequency = activityInfoPtr->sampleFrequency;
    msg->activityData.updateFrequency = activityInfoPtr->updateFrequency;
    msg->activityData.numberOfEntries = activityInfoPtr->numberOfEntries;
    memcpy(&msg->activityData.activities[0], &activityInfoPtr->activities[0], sizeof(uint16)*activityInfoPtr->numberOfEntries);
}
