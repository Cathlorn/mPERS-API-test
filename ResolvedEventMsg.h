#ifndef RESOLVED_EVENT_H
#define RESOLVED_EVENT_H

#include "types.h"
#include "HaloMessage.h"
#include "Location.h"
#include "SignalStrength.h"

enum ResolvedEventType
{
  RESOLVED_FALL = 0,
  RESOLVED_PANIC = 1,
};

//Types of Alerts supported
enum ResolveMethod
{
  MANUAL = 0,
  AUTO   = 1,
  NONE = 2,
};

enum ResolvedType
{
   unknown = 0,
   older_than_time_alarm_sent = 1,
   older_than_last_alarm_reset = 2,
   alarm_already_pending = 3,
   alarm_older_than_pending = 4,
   alarm_send_in_progress = 5,
   alarm_reset_button_pressed = 6,
};

typedef struct
{
    HaloMessage baseMessage;  //12 bytes
    Location currentLocation; //16 bytes for now
    SignalStrength   signalStrength;    //4 bytes
    BatteryInfo battInfo; //8 bytes for now
    uint32   nadTimestamp;      //4 bytes (replaces gw timestamp;nad=Network Access Device)
    uint8    resolvedEventType; //1 byte
    uint8    retryNumber;       //1 byte (Up to 6 tries per upload attempt)
    uint8    resolveMethod;     //1 byte
    uint8    resolveReason;     //1 byte
    uint32   resolvedTimestamp; //4 bytes
}
ResolvedEventMsg;  //Total: 44 bytes

#define RESOLVED_EVENT_INIT(a) { \
                                .baseMessage = HALO_MESSAGE_INIT(RESOLVED_EVENT, 0, 0, 0), \
                                .currentLocation = LOCATION_INIT(0, 0, 0, 0), \
                                .signalStrength = SIGNAL_STRENGTH_INIT(), \
                                .battInfo = BATTERY_INFO_INIT(), \
                                .nadTimestamp = 0, \
                                .resolvedEventType = a, \
                                .retryNumber = 0, \
                                .resolveMethod = 0, \
                                .resolveReason = 0, \
                                .resolvedTimestamp = 0, \
                          }

//Helper Functions

#endif //RESOLVED_EVENT_H
