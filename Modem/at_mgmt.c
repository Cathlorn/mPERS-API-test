#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "at_mgmt.h"
#include "General/str_utils.h"

//Function Prototypes
void mgmt_modem_tx_response_handler(ModemMgmt *pModemMgmt, void * caller, char *responseString);

void enqueueModemCmd(ModemCmdQueue *pModemCmdQueue, ModemCommand modemCommand)
{
    enqueueItem(&(pModemCmdQueue->commandQueue), &modemCommand);
}

void peekModemCmd(ModemCmdQueue *pModemCmdQueue,ModemCommand *pModemCommand)
{
    peekItem(&(pModemCmdQueue->commandQueue), 0, pModemCommand);
}

void dequeueModemCmd(ModemCmdQueue *pModemCmdQueue)
{
    dequeueItem(&(pModemCmdQueue->commandQueue), 0);
}

int queueSizeModemCmd(ModemCmdQueue *pModemCmdQueue)
{
    return queueSize(&(pModemCmdQueue->commandQueue));
}

void mgmt_setup(ModemMgmt *pModemMgmt)
{
}

void mgmt_modem_rx(ModemMgmt *pModemMgmt, void *caller)
{
    static char responseBuffer[1024] = "";
    static int  responseMaxBufferLength = sizeof(responseBuffer);
    static int responseBufferPos = 0;
    int bytesRead = 0;
    char buf[1024];
    int bufMaxLength = sizeof(buf);
    ModemResponseTable *pModemResponseTable = pModemMgmt->pModemResponseTable;

    //assert(uartDevice);
    if (UartRxBytesAvailable(pModemMgmt->pModemDevice) > 0)
    {
        //Shift in each bytes that you read into the string

        if (UartRx(pModemMgmt->pModemDevice, &buf[0], bufMaxLength, &bytesRead) > 0)
        {
            int i;
            char c;

            for (i=0; i < bytesRead; i++)
            {
                c = buf[i];

                if ((c == '\r')||(c == '\n'))
                {
                    //End of Command Detected
                    if (responseBufferPos == 0)
                    {
                        //Empty command detected
                        //Do Nothing
                    }
                    else
                    {
                        int j;

                        mgmt_modem_tx_response_handler(pModemMgmt, caller, responseBuffer);

                        if (pModemResponseTable)
                        {
                            for (j = 0; j < pModemResponseTable->numberOfEntries; j++)
                            {
                                if (strMatchFound(pModemResponseTable->entries[j].response, responseBuffer))
                                {
                                    pModemResponseTable->entries[j].callback(caller, responseBuffer);
                                }
                            }
                        }
                        //printf("response detected: %s\n", responseBuffer);
                    }

                    responseBufferPos = 0;
                    responseBuffer[responseBufferPos] = '\0'; //Makes sure NULL terminated
                }
                else
                {
                    if (responseBufferPos < (responseMaxBufferLength - 1))
                    {
                        responseBuffer[responseBufferPos++] = c;
                        responseBuffer[responseBufferPos] = '\0'; //Makes sure NULL terminated
                    }
                    else
                    {
                        //If sequence exceeds the size of the response buffer, then shift everything left
                        shift_left_text_buffer(responseBuffer, (responseMaxBufferLength - 1), (char *) &c, 1);
                    }
                }
            }
        }
    }
}

void mgmt_modem_tx_response_handler(ModemMgmt *pModemMgmt, void *caller, char *responseString)
{
    ModemCmdQueue *pModemCmdQueue = (pModemMgmt->pModemCmdQueue);

    if (queueSizeModemCmd(pModemCmdQueue) > 0)
    {
        ModemCommand modemCommand = MODEM_COMMAND_DEFAULT_INIT();
        peekModemCmd(pModemCmdQueue, &modemCommand);

        if (pModemMgmt->txState == 1) //Confirm echo
        {
            if (strMatchFound(modemCommand.cmd, responseString)) //Confirm cmd echo
            {
                printf("cmd echo confirmed\n");
                pModemMgmt->txState = 2;
            }
        }
        else if (pModemMgmt->txState == 2) //Search for response
        {
            int j;

            for (j = 0; j < modemCommand.pResponseTable->numberOfEntries; j++)
            {
                if (strMatchFound(modemCommand.pResponseTable->entries[j].response, responseString))
                {
                    printf("cmd response found: %s\n", responseString);
                    if(modemCommand.pResponseTable->entries[j].callback)
                        modemCommand.pResponseTable->entries[j].callback(caller, responseString);
                    pModemMgmt->txState = 0;
                }
            }

            if (pModemMgmt->txState == 0)
            {
                printf("command complete!\n");

                //Free dynamically allocated memory from the command if any is present
                if(modemCommand.freeCmd)
                {
                    free(modemCommand.cmd);
                    modemCommand.cmd = NULL;
                    modemCommand.freeCmd = 0;
                }

                dequeueModemCmd(pModemCmdQueue);
                pModemCmdQueue->ticksRemaining = 0; //force callback
            }
        }
    }
}

void mgmt_modem_tx(ModemMgmt *pModemMgmt, void *caller)
{
    ModemCmdQueue *pModemCmdQueue = (pModemMgmt->pModemCmdQueue);

    if (!pModemCmdQueue->ticksRemaining)
    {
        if (queueSizeModemCmd(pModemCmdQueue) > 0)
        {
            ModemCommand modemCommand = MODEM_COMMAND_DEFAULT_INIT();
            char txCmdBuffer[1024];

            pModemMgmt->txState = 1;
            peekModemCmd(pModemCmdQueue, &modemCommand);
            pModemCmdQueue->tickTimeout = ((modemCommand.secTimeout *1000) / pModemMgmt->msPerTick);
            pModemCmdQueue->ticksRemaining = pModemCmdQueue->tickTimeout;

            sprintf(txCmdBuffer, "%s\r\n", modemCommand.cmd);
            printf("%s\n", txCmdBuffer);
            UartTx(pModemMgmt->pModemDevice, txCmdBuffer, strlen(txCmdBuffer));
        }
    }
    else
    {
        pModemCmdQueue->ticksRemaining--;
    }
}

void mgmt_modem(ModemMgmt *pModemMgmt, void *caller)
{
    //NOTE: This must be run together or locks will be need to protect tx processing since tx is called within rx
    mgmt_modem_tx(pModemMgmt, caller);
    mgmt_modem_rx(pModemMgmt, caller);
}

void addModemCommand(ModemMgmt *pModemMgmt, ModemCommand modemCommand)
{
    enqueueModemCmd((pModemMgmt->pModemCmdQueue), modemCommand);
}

int hasModemCommands(ModemMgmt *pModemMgmt)
{
    ModemCmdQueue *pModemCmdQueue = (pModemMgmt->pModemCmdQueue);

    //return (pModemCmdQueue->numberOfEntries > 0);
    return !isQueueEmpty(&(pModemCmdQueue->commandQueue));
}

int allowModemCommandAdd(ModemMgmt *pModemMgmt)
{
    ModemCmdQueue *pModemCmdQueue = (pModemMgmt->pModemCmdQueue);

    //return (pModemCmdQueue->numberOfEntries < pModemCmdQueue->maxEntries);
    return !isQueueFull(&(pModemCmdQueue->commandQueue));
}
