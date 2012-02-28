#ifndef AT_MGMT_H
#define AT_MGMT_H

#include "at_listener.h"
#include "Uart/posix_uart.h"
#include "General/GenericStaticArrayQueue.h"

typedef struct {
    GenericStaticArrayQueue commandQueue;
    int tickTimeout;
    int ticksRemaining;
}
ModemCmdQueue;

#define MODEM_CMD_QUEUE_INIT(a,b,c) {\
  .commandQueue=GENERIC_STATIC_ARRAY_QUEUE_INIT(a,b,c), \
  .tickTimeout = 0, \
  .ticksRemaining = 0, \
}

typedef struct {
    UartControl *pModemDevice;
    ModemCmdQueue *pModemCmdQueue;
    ModemResponseTable *pModemResponseTable;
    int txState;
    int msPerTick;
}
ModemMgmt;

#define MODEM_MGMT_INIT(a,b,c,d) {\
  .pModemDevice=a, \
  .pModemCmdQueue=b, \
  .pModemResponseTable=c, \
  .txState=0, \
  .msPerTick = d, \
}

#define MODEM_MGMT_DEFAULT_INIT() MODEM_MGMT_INIT(NULL,NULL,NULL,1)

void mgmt_setup(ModemMgmt *pModemMgmt);
void mgmt_modem(ModemMgmt *pModemMgmt, void *caller);
void addModemCommand(ModemMgmt *pModemMgmt, ModemCommand modemCommand);
int hasModemCommands(ModemMgmt *pModemMgmt);
int allowModemCommandAdd(ModemMgmt *pModemMgmt);

#endif //AT_MGMT_H
