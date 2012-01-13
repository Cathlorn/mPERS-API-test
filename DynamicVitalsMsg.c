#include <string.h>

#include "DynamicVitalsMsg.h"

//Populates the structure correctly. Needed since partial xmission is done to save space.
void pack_DynamicVitalsMsg(const DynamicVitalsMsg *msg, void *dstData)
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

    memcpy(dataPtr, &msg->signalStrength, sizeof(SignalStrength));
    dataPtr += sizeof(SignalStrength);

    stepInfoPtr = (StepInfo *) dataPtr;
    stepInfoPtr->sampleRate = msg->stepData.sampleRate;
    stepInfoPtr->numberOfEntries = msg->stepData.numberOfEntries;
    memcpy(&stepInfoPtr->steps[0], &msg->stepData.steps[0], sizeof(uint16)*msg->stepData.numberOfEntries);

    dataPtr = (uint8 *) &stepInfoPtr->steps[0];
    dataPtr += (sizeof(uint16)*stepInfoPtr->numberOfEntries);
    activityInfoPtr = (ActivityInfo *) dataPtr;
    activityInfoPtr->sampleRate = msg->activityData.sampleRate;
    activityInfoPtr->numberOfEntries = msg->activityData.numberOfEntries;
    memcpy(&activityInfoPtr->activities[0], &msg->activityData.activities[0], sizeof(uint16)*msg->activityData.numberOfEntries);
}

void unpack_DynamicVitalsMsg(const void *srcData, DynamicVitalsMsg *msg)
{
    DynamicVitalsMsg *srcMsg = (DynamicVitalsMsg *) srcData;
    StepInfo *stepInfoPtr;
    ActivityInfo *activityInfoPtr;
    uint8 *dataPtr;

    memcpy(&msg->baseMessage, &srcMsg->baseMessage, sizeof(HaloMessage));
    memcpy(&msg->battInfo, &srcMsg->battInfo, sizeof(BatteryInfo));
    memcpy(&msg->currentLocation, &srcMsg->currentLocation, sizeof(Location));
    memcpy(&msg->signalStrength, &srcMsg->signalStrength, sizeof(SignalStrength));

    stepInfoPtr = &srcMsg->stepData;
    msg->stepData.sampleRate = stepInfoPtr->sampleRate;
    msg->stepData.numberOfEntries = stepInfoPtr->numberOfEntries;
    memcpy(&msg->stepData.steps[0], &stepInfoPtr->steps[0], sizeof(uint16)*stepInfoPtr->numberOfEntries);

    dataPtr = (uint8 *) &stepInfoPtr->steps[0];
    dataPtr += (sizeof(uint16)*stepInfoPtr->numberOfEntries);
    activityInfoPtr = (ActivityInfo *) dataPtr;
    msg->activityData.sampleRate = activityInfoPtr->sampleRate;
    msg->activityData.numberOfEntries = activityInfoPtr->numberOfEntries;
    memcpy(&msg->activityData.activities[0], &activityInfoPtr->activities[0], sizeof(uint16)*activityInfoPtr->numberOfEntries);
}
