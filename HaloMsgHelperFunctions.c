#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "HaloMsgHelperFunctions.h"
#include "HaloMessageTypes.h"
#include "BatteryInfo.h"
#include "Location.h"
#include "PanicMsg.h"

void init_base_message(uint16 commandType, uint8 formatVersion,
                       uint32 timeStamp, uint32 deviceId,
                       HaloMessage *message)
{
    message->commandType = commandType;
    message->formatVersion = formatVersion;
    message->time = timeStamp;
    message->deviceId = deviceId;
}

#define MAX_STEPS 50

static uint16 stepBuffer[MAX_STEPS];

#define MAX_ACTIVITY 50

static uint16 activityBuffer[MAX_ACTIVITY];

int StepInfo_static_alloc(uint16 *steps, int numberOfSteps, StepInfo *stepInfo)
{
    int success = -1;

    if ((numberOfSteps < MAX_STEPS)&&(numberOfSteps >= 0))
    {
        //stepInfo->steps = stepBuffer;
        stepInfo->numberOfEntries = numberOfSteps;
        memcpy(stepInfo->steps, steps, numberOfSteps*sizeof(uint16));
        success = 0;
    }

    return success;
}

int StepInfo_static_free(StepInfo *stepInfo)
{
    int success = -1;

    success = 0;

    return success;
}

int ActivityInfo_static_alloc(uint16 *activities, int numberOfActivities, ActivityInfo *activityInfo)
{
    int success = -1;

    if ((numberOfActivities < MAX_ACTIVITY)&&(numberOfActivities >= 0))
    {
        //activityInfo->activities = activityBuffer;
        activityInfo->numberOfEntries = numberOfActivities;
        memcpy(activityInfo->activities, activities, numberOfActivities*sizeof(uint16));
        success = 0;
    }

    return success;
}

int ActivityInfo_static_free(ActivityInfo *activityInfo)
{
    int success = -1;

    success = 0;

    return success;
}

int getMsgLength(const HaloMessage *msg)
{
    int len = -1;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        DynamicVitals dynamicVitalsMsg;

        unpack_DynamicVitals(msg, &dynamicVitalsMsg);

        len  = sizeof(HaloMessage);
        len += sizeof(BatteryInfo);
        len += sizeof(Location);
        len += (4 + (2*dynamicVitalsMsg.stepData.numberOfEntries));
        len += (4 + (2*dynamicVitalsMsg.activityData.numberOfEntries));
    }
    else if(msg->commandType == CRITICAL_ALERT)
    {
        CriticalAlert *criticalAlertMsg = (CriticalAlert *) msg;

        if(criticalAlertMsg->criticalAlertType == FALL)
        {
            printf("Format (%d) presently not supported.\n", criticalAlertMsg->criticalAlertType);
            assert(0);
        }
        else if(criticalAlertMsg->criticalAlertType == PANIC)
        {
            len = sizeof(PanicMsg);
        }
        else if(criticalAlertMsg->criticalAlertType == CLEAR)
        {
            printf("Format (%d) presently not supported.\n", criticalAlertMsg->criticalAlertType);
            assert(0);
        }
        else
        {
            printf("Invalid Alert Type: %d\n", criticalAlertMsg->criticalAlertType);
            assert(0);
        }
    }

    return len;
}
