#ifndef HALO_MSG_HELPER_FUNCTIONS_H
#define HALO_MSG_HELPER_FUNCTIONS_H

#include "DynamicVitals.h"
#include "StepInfo.h"
#include "ActivityInfo.h"
#include "HaloMessage.h"

void init_base_message(uint16 commandType, uint8 formatVersion,
                       uint32 timeStamp, uint32 deviceId,
                       HaloMessage *message);

int StepInfo_static_alloc(uint16 *steps, int numberOfSteps, StepInfo *stepInfo);
int StepInfo_static_free(StepInfo *stepInfo);

int ActivityInfo_static_alloc(uint16 *activities, int numberOfActivities, ActivityInfo *activityInfo);
int ActivityInfo_static_free(ActivityInfo *activityInfo);

int getMsgLength(const HaloMessage *msg);

#endif //HALO_MSG_HELPER_FUNCTIONS_H
