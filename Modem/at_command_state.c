#include "at_command_state.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "General/str_utils.h"

#define SUCCESS 0
#define FAIL    -1

void testUart(void)
{
    UartControl uartControl = UART_CONTROL_INIT("/dev/ttyUSB0");
    char buff[1024];
    int bytesRead;

    if (configureUart(&uartControl, PROFILE_56000_8N1, 1) == SUCCESS)
    {
        if (UartTx(&uartControl, "\n", 1) > 0)
        {
            sleep(1);
            UartRx(&uartControl, buff, sizeof(buff) - 1, &bytesRead);
            if (bytesRead > 0)
            {
                buff[bytesRead] = '\0';
                printf("bytesRead: %s\n", buff);
            }
        }
    }
}

//Modem controls
//static UartControl atModemDevice = UART_CONTROL_INIT("/dev/ttyUSB0");
static UartControl atModemDevice = UART_CONTROL_INIT("/dev/ttyACM0");
void setupModem(void)
{
    assert(configureUart(&atModemDevice, PROFILE_56000_8N1, 1) == SUCCESS);
}

int testAT(void)
{
    int passed = 0;
    const char testATCommand[] = "AT\r\n";
    const char testATAcknowledgeString[] = "OK";
    int timeout = 500; //5s (in 10 ms increments)
    int buffOffset = 0;
    char buff[1024];
    int maxBuffLength = sizeof(buff);
    int bytesRead = -1;

    if (UartTx(&atModemDevice, testATCommand, strlen(testATCommand)) > 0)
    {
        while (timeout--)
        {
            if (UartRxBytesAvailable(&atModemDevice) > 0)
            {
                //Shift in each bytes that you read into the string
                maxBuffLength = sizeof(buff) -buffOffset - 1;
                if (maxBuffLength <= 0)
                {
                    break;
                }
                else if (UartRx(&atModemDevice, &buff[buffOffset], maxBuffLength, &bytesRead) > 0)
                {
                    buffOffset += bytesRead;
                    buff[buffOffset] = '\0';

                    if (strMatchFound((char *) testATAcknowledgeString, (char *) buff))
                    {
                        passed = 1;
                        break;
                    }
                }
            }
            usleep(10000);
        }
    }

    return passed;
}

void flushRead(ATCommandState *atCommandState)
{
    char buf[1024];
    int bytesRead = 0;

    while (UartRxBytesAvailable(&atCommandState->modemDevice) > 0)
    {
        UartRx(&atCommandState->modemDevice, &buf[0], sizeof(buf), &bytesRead);
    }
}

/*int TxATCmd_start(ATCommandState *atCommandState, const char *atCmd, const int atCmdLength)
{
    int passed = 0;

    assert(atCommandState);

    atCommandState->txOperation.buffer = (char *) atCmd;
    atCommandState->txOperation.bufferLength = atCmdLength;
    atCommandState->txOperation.busy = 1;

    //Flush everything before the operation starts
    //fflush(atCommandState->modemDevice.uartFd);
    flushRead(atCommandState);
    atCommandState->txOperation.bytesTransferred = UartTx(&atCommandState->modemDevice, atCmd, atCmdLength);

    if (atCommandState->txOperation.bytesTransferred > 0)
    {
        atCommandState->rxOperation.ticksRemaining = 500; // 500, 10 ms ticks (5 s timeout)
        atCommandState->rxOperation.busy = 1;
        atCommandState->txOperation.busy = 0; //placed here to prevent a hazard if running reporting
        passed = 1;
    }

    return passed;
}

void TxATCmd_tick(ATCommandState *atCommandState)
{
    const char testATAcknowledgeString[] = "OK";
    int maxBuffLength;
    int bytesRead;

    assert(atCommandState);

    if (atCommandState->rxOperation.busy)
    {
        if (atCommandState->rxOperation.ticksRemaining)
        {
            atCommandState->rxOperation.ticksRemaining--;
            if (UartRxBytesAvailable(&atCommandState->modemDevice) > 0)
            {
                //Shift in each bytes that you read into the string
                maxBuffLength = atCommandState->rxOperation.bufferLength - atCommandState->rxOperation.bytesTransferred - 1;
                if (maxBuffLength <= 0)
                {
                    atCommandState->rxOperation.busy = 0;
                }
                else if (UartRx(&atCommandState->modemDevice, &atCommandState->rxOperation.buffer[atCommandState->rxOperation.bytesTransferred], maxBuffLength, &bytesRead) > 0)
                {
                    atCommandState->rxOperation.bytesTransferred += bytesRead;
                    atCommandState->rxOperation.buffer[atCommandState->rxOperation.bytesTransferred] = '\0';

                    if (strMatchFound((char *) testATAcknowledgeString, (char *) atCommandState->rxOperation.buffer))
                    {
                        atCommandState->rxOperation.busy = 0;
                    }
                }
            }
        }
        else
        {
            //Process failure
            atCommandState->rxOperation.busy = 0;
        }
    }
}

int TxATCmd_running(ATCommandState *atCommandState)
{
    assert(atCommandState);

    return (atCommandState->txOperation.busy || atCommandState->rxOperation.busy);
} */

/*int TxATCmd(const char *atCmd, const int atCmdLength)
{
    int success = 0;
    ATCommandState atCommandState = AT_COMMAND_STATE_INIT();
    const char testATAcknowledgeString[] = "OK";
    char rxBuf[1024] = {0};

    atCommandState.txOperation.buffer = atCmd;
    atCommandState.txOperation.bufferLength = atCmdLength;
    atCommandState.rxOperation.buffer = rxBuf;
    atCommandState.rxOperation.bufferLength = sizeof(rxBuf);
    atCommandState.modemDevice = atModemDevice;

    if (TxATCmd_start(&atCommandState, atCmd, atCmdLength))
    {
        while (TxATCmd_running(&atCommandState))
        {
            TxATCmd_tick(&atCommandState);
            usleep(10000);
        }

        if (atCommandState.rxOperation.bytesTransferred > 0)
        {
            printf("AT Result: %s\n", atCommandState.rxOperation.buffer);

            if (strMatchFound((char *) testATAcknowledgeString, (char *) atCommandState.rxOperation.buffer))
            {
                success = 1;
            }
        }
    }

    return success;
} */

/*int TxATCmd_start(ATCommandState *atCommandState, const char *atCmd, const int atCmdLength)
{
    int passed = 0;

    assert(atCommandState);

    atCommandState->txOperation.buffer = (char *) atCmd;
    atCommandState->txOperation.bufferLength = atCmdLength;
    atCommandState->txOperation.busy = 1;

    //Flush everything before the operation starts
    //fflush(atCommandState->modemDevice.uartFd);
    flushRead(atCommandState);
    atCommandState->txOperation.bytesTransferred = UartTx(&atCommandState->modemDevice, atCmd, atCmdLength);

    if (atCommandState->txOperation.bytesTransferred > 0)
    {
        atCommandState->rxOperation.ticksRemaining = 500; // 500, 10 ms ticks (5 s timeout)
        atCommandState->rxOperation.busy = 1;
        atCommandState->txOperation.busy = 0; //placed here to prevent a hazard if running reporting
        passed = 1;
    }

    return passed;
}

void TxATCmd_tick(ATCommandState *atCommandState)
{
    const char testATAcknowledgeString[] = "OK";
    int maxBuffLength;
    int bytesRead;

    assert(atCommandState);

    if (atCommandState->rxOperation.busy)
    {
        if (atCommandState->rxOperation.ticksRemaining)
        {
            atCommandState->rxOperation.ticksRemaining--;
            if (UartRxBytesAvailable(&atCommandState->modemDevice) > 0)
            {
                //Shift in each bytes that you read into the string
                maxBuffLength = atCommandState->rxOperation.bufferLength - atCommandState->rxOperation.bytesTransferred - 1;
                if (maxBuffLength <= 0)
                {
                    atCommandState->rxOperation.busy = 0;
                }
                else if (UartRx(&atCommandState->modemDevice, &atCommandState->rxOperation.buffer[atCommandState->rxOperation.bytesTransferred], maxBuffLength, &bytesRead) > 0)
                {
                    atCommandState->rxOperation.bytesTransferred += bytesRead;
                    atCommandState->rxOperation.buffer[atCommandState->rxOperation.bytesTransferred] = '\0';

                    if (strMatchFound((char *) testATAcknowledgeString, (char *) atCommandState->rxOperation.buffer))
                    {
                        atCommandState->rxOperation.busy = 0;
                    }
                }
            }
        }
        else
        {
            //Process failure
            atCommandState->rxOperation.busy = 0;
        }
    }
}

int TxATCmd_running(ATCommandState *atCommandState)
{
    assert(atCommandState);

    return (atCommandState->txOperation.busy || atCommandState->rxOperation.busy);
} */

int TxATCmd_start(ATCommandState *atCommandState)
{
    int passed = 0;

    assert(atCommandState);

    atCommandState->commandComplete = 0;
    atCommandState->txOperation.busy = 1;

    //Flush everything before the operation starts
    //fflush(atCommandState->modemDevice.uartFd);
    flushRead(atCommandState);
    atCommandState->txOperation.bytesTransferred = UartTx(&atCommandState->modemDevice, atCommandState->txOperation.buffer, atCommandState->txOperation.bufferLength);

    if (atCommandState->txOperation.bytesTransferred > 0)
    {
        atCommandState->rxOperation.busy = 1;
        atCommandState->txOperation.busy = 0; //placed here to prevent a hazard if running reporting
        passed = 1;
    }

    return passed;
}

void TxATCmd_tick(ATCommandState *atCommandState)
{
    int maxBuffLength;
    int bytesRead;

    assert(atCommandState);

    if (atCommandState->rxOperation.busy)
    {
        if (atCommandState->rxOperation.ticksRemaining)
        {
            atCommandState->rxOperation.ticksRemaining--;
            if (UartRxBytesAvailable(&atCommandState->modemDevice) > 0)
            {
                //Shift in each bytes that you read into the string
                maxBuffLength = atCommandState->rxOperation.bufferLength - atCommandState->rxOperation.bytesTransferred - 1;
                if (maxBuffLength <= 0)
                {
                    atCommandState->rxOperation.busy = 0;
                }
                else if (UartRx(&atCommandState->modemDevice, &atCommandState->rxOperation.buffer[atCommandState->rxOperation.bytesTransferred], maxBuffLength, &bytesRead) > 0)
                {
                    atCommandState->rxOperation.bytesTransferred += bytesRead;
                    atCommandState->rxOperation.buffer[atCommandState->rxOperation.bytesTransferred] = '\0';

                    if (strMatchFound(atCommandState->expectedResponseString, (char *) atCommandState->rxOperation.buffer))
                    {
                        atCommandState->commandComplete = 1;
                        atCommandState->rxOperation.busy = 0;
                    }
                }
            }
        }
        else
        {
            //Process failure
            atCommandState->rxOperation.busy = 0;
        }
    }
}

int TxATCmd_running(ATCommandState *atCommandState)
{
    assert(atCommandState);

    return (atCommandState->txOperation.busy || atCommandState->rxOperation.busy);
}

int TxATCmd(const char *atCmd, const int atCmdLength,
            const char *expectedRespString, const int expectedRespStringLength,
            int secTimeout)
{
    int success = 0;
    ATCommandState atCommandState = AT_COMMAND_STATE_INIT();
    char rxBuf[1024] = {0};
    int usWait = 10000;
    int tickTimeout = ((secTimeout*1000) / (usWait/1000));

    atCommandState.txOperation.buffer = (char *) atCmd;
    atCommandState.txOperation.bufferLength = atCmdLength;
    atCommandState.txOperation.ticksRemaining = tickTimeout;
    atCommandState.rxOperation.buffer = (char *) rxBuf;
    atCommandState.rxOperation.bufferLength = sizeof(rxBuf);
    atCommandState.rxOperation.ticksRemaining = tickTimeout;
    atCommandState.expectedResponseString = (char *) expectedRespString;
    atCommandState.expectedResponseStringLength = expectedRespStringLength;
    atCommandState.modemDevice = atModemDevice;

    if (TxATCmd_start(&atCommandState))
    {
        while (TxATCmd_running(&atCommandState))
        {
            TxATCmd_tick(&atCommandState);
            usleep(usWait);
        }

        if (atCommandState.commandComplete)
        {
            success = 1;
        }

        if (atCommandState.rxOperation.bytesTransferred > 0)
        {
            printf("AT Result: %s\n", atCommandState.rxOperation.buffer);
        }
    }

    return success;
}

