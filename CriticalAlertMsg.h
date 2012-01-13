#ifndef CRITICAL_ALERT_H
#define CRITICAL_ALERT_H

#include "types.h"
#include "HaloMessage.h"
#include "Location.h"
#include "SignalStrength.h"

//Types of Alerts supported
enum CriticalAlertType
{
  FALL = 0,
  PANIC = 1,
  OPERATOR_ACK = 2,
};

typedef struct
{
    HaloMessage baseMessage;  //12 bytes
    Location currentLocation; //16 bytes for now
    SignalStrength   signalStrength;    //4 bytes
    BatteryInfo battInfo; //8 bytes for now
    uint32   nadTimestamp;      //4 bytes (replaces gw timestamp;nad=Network Access Device)
    uint8    criticalAlertType; //1 byte
    uint8    retryNumber;       //1 byte (Up to 6 tries per upload attempt)
}
CriticalAlertMsg;  //Total: 44 bytes

#define CRITICAL_ALERT_MSG_INIT(a) { \
                                .baseMessage = HALO_MESSAGE_INIT(CRITICAL_ALERT, 0, 0, 0), \
                                .currentLocation = LOCATION_INIT(0, 0, 0, 0), \
                                .signalStrength = SIGNAL_STRENGTH_INIT(), \
                                .battInfo = BATTERY_INFO_INIT(), \
                                .nadTimestamp = 0, \
                                .criticalAlertType = a, \
                                .retryNumber = 0, \
                          }

//Helper Functions

#endif //CRITICAL_ALERT_H
