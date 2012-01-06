#ifndef PANIC_MSG_H
#define PANIC_MSG_H

#include "types.h"
#include "CriticalAlert.h"

typedef struct
{
    CriticalAlert criticalAlertBaseMsg; //44 bytes
    uint16 durationPress;               //2 bytes
    uint8 reserved[2];                  //2 bytes
}
PanicMsg; //48 bytes

#define PANIC_MESSAGE_INIT() { \
                            .criticalAlertBaseMsg = CRITICAL_ALERT_INIT(PANIC), \
                            .durationPress = 0, \
                            .reserved = {0}, \
                          }

#endif //PANIC_MSG_H
