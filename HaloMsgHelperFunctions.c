#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "HaloMsgHelperFunctions.h"
#include "HaloMessageTypes.h"
#include "BatteryInfo.h"
#include "Location.h"
#include "PanicMsg.h"
#include "DynamicVitals.h"

void init_base_message(uint16 commandType, uint8 formatVersion,
                       uint32 timeStamp, uint32 deviceId,
                       HaloMessage *message)
{
    message->commandType = commandType;
    message->formatVersion = formatVersion;
    message->time = timeStamp;
    message->deviceId = deviceId;
}

int getMsgLength(const HaloMessage *msg)
{
    int len = -1;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        DynamicVitals dynamicVitalsMsg;

        unpack_DynamicVitals( (const void *) msg, (DynamicVitals *) &dynamicVitalsMsg);

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
