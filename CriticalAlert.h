#ifndef CRITICAL_ALERT_H
#define CRITICAL_ALERT_H

#include "types.h"
#include "HaloMessage.h"
#include "Location.h"

enum CriticalAlertType
{
  FALL = 0,
  PANIC = 1,
  CLEAR = 2,
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
    uint32   nadTimestamp;      //4 bytes (replaces gw timestamp;nad=Network Access Device)
    uint32   signalStrength;    //4 bytes
    uint8    criticalAlertType; //1 byte
    uint8    retryNumber;       //1 byte (Up to 6 tries per upload attempt)
    uint8    resolveMethod;     //1 byte
    uint8    resolveReason;     //1 byte
    uint32   resolvedTimestamp; //4 bytes
}
CriticalAlert;  //Total: 44 bytes

//Helper Functions

#endif //CRITICAL_ALERT_H
