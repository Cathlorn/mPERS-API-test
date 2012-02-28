#ifndef AT_COMMAND_STATE_H
#define AT_COMMAND_STATE_H

#include "Uart/posix_uart.h"

typedef struct
{
    //Processing
    int ticksRemaining;
    char *buffer;
    int bufferLength;
    int busy;

    //Result
    int bytesTransferred; //could be sent or received
}
OperationData;

#define OPERATION_DATA_INIT() {\
  .ticksRemaining=0, \
  .buffer = NULL, \
  .bufferLength=0, \
  .busy=0, \
  .bytesTransferred=0, \
} \

typedef struct
{
    UartControl modemDevice;
    OperationData txOperation;
    OperationData rxOperation;
    char *expectedResponseString;
    int expectedResponseStringLength;
    int commandComplete;
}
ATCommandState;

#define AT_COMMAND_STATE_INIT() {\
  .modemDevice = UART_CONTROL_INIT(""), \
  .txOperation=OPERATION_DATA_INIT(), \
  .rxOperation=OPERATION_DATA_INIT(), \
  .expectedResponseString="", \
  .expectedResponseStringLength=1, \
  .commandComplete=0, \
} \

void flushRead(ATCommandState *atCommandState);
int TxATCmd_start(ATCommandState *atCommandState);
void TxATCmd_tick(ATCommandState *atCommandState);
int TxATCmd_running(ATCommandState *atCommandState);

int TxATCmd(const char *atCmd, const int atCmdLength,
            const char *expectedRespString, const int expectedRespStringLength,
            int secTimeout);

void setupModem(void);

#endif //AT_COMMAND_STATE_H
