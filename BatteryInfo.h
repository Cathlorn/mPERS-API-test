#ifndef BATTERY_INFO_H
#define BATTERY_INFO_H

#include "types.h"

enum BattStatus
{
    NORMAL,
    LOW_BATT,
    EXTREME_LOW_BATT,
    SHUTDOWN
};

typedef struct _BatteryInfo
{
    uint16 timeRemaining;     //How many secs of batt life left
    uint8  battPercentage;    //We might want this based on time remaining / time expected
    uint8  unusedBits : 2;
    uint8  status : 3;        //Status(Normal,Low Batt,Extremely Low Batt,Shutdown/Dying Gasp)
    uint8  chargeComplete : 1;//Tells if charge ever completed. This is a sticky bit that
    //clears when charging and sets when charging completes.
    uint8  charging : 1;      //Tells if charging
    uint8  plugged  : 1;      //Tells if plugged in
    //uint8  unusedBits : 2;
}
BatteryInfo;

#define BATTERY_INFO_INIT() { \
 .timeRemaining  = 0, \
 .battPercentage = 0, \
 .unusedBits = 0, \
 .status = 0, \
 .chargeComplete = 0, \
 .charging = 0, \
 .plugged = 0, \
}


#endif //BATTERY_INFO_H
