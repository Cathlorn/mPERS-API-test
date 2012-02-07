#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "HaloMsgHelperFunctions.h"
#include "HaloMessageTypes.h"
#include "BatteryInfo.h"
#include "Location.h"
#include "DynamicVitalsMsg.h"
#include "CriticalAlertMsg.h"
#include "ResolvedEventMsg.h"

void init_base_message(uint16 commandType, uint8 formatVersion,
                       uint32 timeStamp, uint32 deviceId,
                       HaloMessage *message)
{
    message->commandType = commandType;
    message->formatVersion = formatVersion;
    message->time = timeStamp;
    message->deviceId = deviceId;
}

/*int getMsgLength(const HaloMessage *msg)
{
    int len = -1;

    if (msg->commandType == ALL_DATA_DYNAMIC)
    {
        DynamicVitalsMsg dynamicVitalsMsg;

        unpack_DynamicVitalsMsg( (const void *) msg, (DynamicVitalsMsg *) &dynamicVitalsMsg);

        len  = sizeof(HaloMessage);
        len += sizeof(BatteryInfo);
        len += sizeof(Location);
        len += sizeof(SignalStrength);
        len += (4 + (2*dynamicVitalsMsg.stepData.numberOfEntries));
        len += (4 + (2*dynamicVitalsMsg.activityData.numberOfEntries));
    }
    else if(msg->commandType == CRITICAL_ALERT)
    {
        CriticalAlertMsg *criticalAlertMsg = (CriticalAlertMsg *) msg;

        if((criticalAlertMsg->criticalAlertType == FALL)||
           (criticalAlertMsg->criticalAlertType == PANIC) ||
           (criticalAlertMsg->criticalAlertType == OPERATOR_ACK))
        {
           len = sizeof(CriticalAlertMsg);
        }
        else
        {
            printf("Invalid Alert Type: %d\n", criticalAlertMsg->criticalAlertType);
            assert(0);
        }
    }
    else if(msg->commandType == RESOLVED_EVENT)
    {
        ResolvedEventMsg *resolvedEventMsg = (ResolvedEventMsg *) msg;

        if((resolvedEventMsg->resolvedEventType == RESOLVED_FALL)||
           (resolvedEventMsg->resolvedEventType == RESOLVED_PANIC))
        {
           len = sizeof(ResolvedEventMsg);
        }
        else
        {
            printf("Invalid Alert Type: %d\n", resolvedEventMsg->resolvedEventType);
            assert(0);
        }
    }

    return len;
} */
