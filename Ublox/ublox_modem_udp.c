#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

//Includes
#include "Modem/at_mgmt.h"
#include "General/str_utils.h"
#include "General/GenericStaticArrayQueue.h"
#include "ublox_str_utils.h"
#include "ublox_modem_udp.h"

//Function Prototypes
void UbloxUdpDataConnectSuccessCallback( void * caller, char * responseString );
void UbloxUdpDataConnectFailCallback( void * caller, char * responseString );
void UbloxUdpDataSendCallback( void * caller, char * responseString );
void UbloxUdpDataAvailableCallback( void * caller, char * responseString );
void UbloxUdpDataRecvCallback( void * caller, char * responseString );
void UbloxPdpDeactivatedCallback( void * caller, char * responseString );
void UbloxIpLookupCallback( void * caller, char * responseString );

ModemResponseEntry defaultModemResponseEntries[] =
{
    {"OK", NULL},
};
ModemResponseTable defaultModemResponseTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(defaultModemResponseEntries);

ModemResponseEntry allowErrorModemResponseEntries[] =
{
    {"OK", NULL},
    {"ERROR", NULL},
};
ModemResponseTable allowErrorModemResponseTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(allowErrorModemResponseEntries);

ModemResponseEntry udpConnectProcessEntries[] =
{
    {"OK", UbloxUdpDataConnectSuccessCallback},
    {"ERROR", UbloxUdpDataConnectFailCallback},
};
ModemResponseTable udpConnectProcessTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(udpConnectProcessEntries);

ModemResponseEntry udpSendProcessEntries[] =
{
    {"+UPSND", UbloxUdpDataSendCallback},
};
ModemResponseTable udpSendProcessTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(udpSendProcessEntries);

ModemResponseEntry asyncModemResponseEntries[] =
{
    {"+UUSORD", UbloxUdpDataAvailableCallback},
    {"+UUSORF", UbloxUdpDataAvailableCallback},
    {"+UUPSDD", UbloxPdpDeactivatedCallback},
};
ModemResponseTable asyncModemResponseTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(asyncModemResponseEntries);

ModemResponseEntry udpRecvProcessEntries[] =
{
    {"+USORF", UbloxUdpDataRecvCallback},
};
ModemResponseTable udpRecvProcessTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(udpRecvProcessEntries);

ModemResponseEntry ipLookupEntries[] =
{
    {"+UDNSRN", UbloxIpLookupCallback},
};
ModemResponseTable ipLookupTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(ipLookupEntries);

ModemCommand enableModemCommands[] =
{
    MODEM_COMMAND_INIT("AT+CFUN=1", &defaultModemResponseTable, 5),
};

void EnableModem(ModemMgmt *pModemMgmt)
{
    int i;

    for (i = 0; i < (sizeof(enableModemCommands)/sizeof(ModemCommand)); i++)
    {
        addModemCommand(pModemMgmt, enableModemCommands[i]);
    }
}

ModemCommand connectCellNetworkCommands[] =
{
    MODEM_COMMAND_INIT("AT+CREG=1", &defaultModemResponseTable, 30),
};

void ConnectCellNetwork(ModemMgmt *pModemMgmt)
{
    int i;

    for (i = 0; i < (sizeof(connectCellNetworkCommands)/sizeof(ModemCommand)); i++)
    {
        addModemCommand(pModemMgmt, connectCellNetworkCommands[i]);
    }
}

ModemCommand setupDataProfileCommands[] =
{
    MODEM_COMMAND_INIT("AT+UPSDA=1,4", &allowErrorModemResponseTable, 5),
    MODEM_COMMAND_INIT("AT+UDCONF=1,1", &defaultModemResponseTable, 5),
    MODEM_COMMAND_INIT("AT+UPSD=1,1,\"epc.tmobile.com\"", &defaultModemResponseTable, 5),
    MODEM_COMMAND_INIT("AT+UPSDA=1,1", &defaultModemResponseTable, 5),
    MODEM_COMMAND_INIT("AT+UPSDA=1,3", &defaultModemResponseTable, 5),
    MODEM_COMMAND_INIT("AT+UPSND=1,0", &defaultModemResponseTable, 5),
};

void SetupDataProfile(UbloxModemUdpStruct *pUbloxModemUdpStruct)
{
    int i;
    ModemMgmt *pModemMgmt = pUbloxModemUdpStruct->pModemMgmt;
    char *dataCheckCmd = (char *) malloc(2048);
    ModemCommand dataCheckCommand = MODEM_COMMAND_INIT(dataCheckCmd, &udpConnectProcessTable, 5);

    dataCheckCommand.freeCmd = 1;
    sprintf(dataCheckCmd, "AT+USORF=0,0");

    for (i = 0; i < (sizeof(setupDataProfileCommands)/sizeof(ModemCommand)); i++)
    {
        addModemCommand(pModemMgmt, setupDataProfileCommands[i]);
    }
    addModemCommand(pModemMgmt, dataCheckCommand);
}

void InitiateModemRead(ModemMgmt *pModemMgmt, int len)
{
    //char readCmd[2048] = "";
    char *readCmd = (char *) malloc(2048);
    ModemCommand readDataCommand = MODEM_COMMAND_INIT(readCmd, &udpRecvProcessTable, 5);

    readDataCommand.freeCmd = 1;
    sprintf(readCmd, "AT+USORF=0,%d",
            len);
    printf("readCmd: %s\n", readCmd);

    addModemCommand(pModemMgmt, readDataCommand);
}

void ModemSendTo(ModemMgmt *pModemMgmt, GenericIP socketAddress, unsigned char *data, int len)
{
    //char sendCmd[2048] = "";
    char *sendCmd = (char *) malloc(2048);
    char dataString[1500] = "";
    char ipString[200];
    ModemCommand sendDataCommand = MODEM_COMMAND_INIT(sendCmd, &allowErrorModemResponseTable, 5);

    GetIpString(socketAddress.address, ipString, sizeof(ipString));
    createUbloxHexString(data, len, dataString, sizeof(dataString));

    sendDataCommand.freeCmd = 1;
    sprintf(sendCmd, "AT+USOST=0,\"%s\",%d,%d,\"%s\"",
            ipString, socketAddress.port, len, dataString);
    //sprintf(sendCmd, "AT+USOST=0,\"ec2-204-236-192-230.compute-1.amazonaws.com\",%d,%d,\"%s\"",
    //        socketAddress.port, len, dataString);
    printf("sendCmd: %s\n", sendCmd);

    addModemCommand(pModemMgmt, sendDataCommand);
}

int ModemRecv(UbloxModemUdpStruct *pUbloxModemUdpStruct, unsigned char *data, int max_len, GenericIP *sourceAddress)
{
    int bytesRead = 0;
    RxData rxData = RX_DATA_INIT();

    if (!isQueueEmpty(pUbloxModemUdpStruct->pRxQueue))
    {
        peekItem(pUbloxModemUdpStruct->pRxQueue, 0, &rxData);

        bytesRead = (rxData.len < max_len) ? rxData.len : max_len;
        memcpy(data, rxData.data, bytesRead);

        *sourceAddress = rxData.sourceAddress;

        dequeueItem(pUbloxModemUdpStruct->pRxQueue, 0);
    }

    return bytesRead;
}

ModemCommand removeDataProfileCommands[] =
{
    MODEM_COMMAND_INIT("AT+USOCL=0", &defaultModemResponseTable, 2),
    MODEM_COMMAND_INIT("AT+UPSDA=1,4", &defaultModemResponseTable, 2),
};

void RemoveDataProfile(ModemMgmt *pModemMgmt)
{
    int i;

    for (i = 0; i < (sizeof(removeDataProfileCommands)/sizeof(ModemCommand)); i++)
    {
        addModemCommand(pModemMgmt, removeDataProfileCommands[i]);
    }
}

ModemCommand disconnectCellNetworkCommands[] =
{
    MODEM_COMMAND_INIT("AT+CREG=0", &defaultModemResponseTable, 30),
};

void DisconnectCellNetwork(ModemMgmt *pModemMgmt)
{
    int i;

    for (i = 0; i < (sizeof(disconnectCellNetworkCommands)/sizeof(ModemCommand)); i++)
    {
        addModemCommand(pModemMgmt, disconnectCellNetworkCommands[i]);
    }
}

ModemCommand disableModemCommands[] =
{
    MODEM_COMMAND_INIT("AT+CFUN=0", &defaultModemResponseTable, 5),
};

void DisableModem(ModemMgmt *pModemMgmt)
{
    int i;

    for (i = 0; i < (sizeof(disableModemCommands)/sizeof(ModemCommand)); i++)
    {
        addModemCommand(pModemMgmt, disableModemCommands[i]);
    }
}

typedef enum
{
    UDP_STR_DECODE_FIND_ECHO,
    UDP_STR_DECODE_GET_SOCKET,
    UDP_STR_DECODE_GET_SERVER_ADDRESS,
    UDP_STR_DECODE_GET_SERVER_PORT,
    UDP_STR_DECODE_GET_DATA_LENGTH,
    UDP_STR_DECODE_GET_DATA
}
UdpRecvStringDecodeState;

void DecodeUbloxUdpRecvString(UbloxModemUdpStruct *pUbloxModemUdpStruct, char *responseString)
{
    //Recv: +USORF: <socket>,”<SERVER_IP>”,<SERVER_PORT>,<length read>,”<hex string of rx bytes>"
    int len = strlen(responseString);
    UdpRecvStringDecodeState currentDecodeState = UDP_STR_DECODE_FIND_ECHO;
    char argumentString[1024] = "";
    int socket = -1;
    char serverAddress[1024] = "";
    int serverPort = -1;
    int dataLen = 0;
    unsigned char data[2048] = {0};
    int i;
    int argumentStringOffset = 0;
    int notifyOfNewData = 0;

    //Process each character in the response string
    for (i = 0; i < len; i++)
    {
        if (currentDecodeState == UDP_STR_DECODE_FIND_ECHO)
        {
            if (strMatchFound(argumentString, "+USORF:"))
            {
                currentDecodeState = UDP_STR_DECODE_GET_SOCKET;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == UDP_STR_DECODE_GET_SOCKET)
        {
            if (responseString[i] == ',')
            {
                socket = atoi(argumentString);
                currentDecodeState = UDP_STR_DECODE_GET_SERVER_ADDRESS;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == UDP_STR_DECODE_GET_SERVER_ADDRESS)
        {
            if (responseString[i] == ',')
            {
                strcpy(serverAddress, argumentString);
                currentDecodeState = UDP_STR_DECODE_GET_SERVER_PORT;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == UDP_STR_DECODE_GET_SERVER_PORT)
        {
            if (responseString[i] == ',')
            {
                serverPort = atoi(argumentString);
                currentDecodeState = UDP_STR_DECODE_GET_DATA_LENGTH;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == UDP_STR_DECODE_GET_DATA_LENGTH)
        {
            if (responseString[i] == ',')
            {
                dataLen = atoi(argumentString);
                currentDecodeState = UDP_STR_DECODE_GET_DATA;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == UDP_STR_DECODE_GET_DATA)
        {
            if ((responseString[i] == '\r')||(responseString[i] == '\n'))
            {
                decodeUbloxHexString(argumentString, strlen(argumentString), data, sizeof(data));
                currentDecodeState = UDP_STR_DECODE_FIND_ECHO;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
                notifyOfNewData = 1;
            }
            else if (responseString[i] == '\"')
            {
                //Do Nothing
                //Ignore these characters
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
    }

    //Handles Termination through end of input
    if (currentDecodeState == UDP_STR_DECODE_GET_DATA)
    {
        decodeUbloxHexString(argumentString, strlen(argumentString), data, sizeof(data));
        currentDecodeState = UDP_STR_DECODE_FIND_ECHO;
        argumentStringOffset = 0;
        argumentString[argumentStringOffset] = '\0';
        notifyOfNewData = 1;
    }

    if (notifyOfNewData)
    {
        RxData rxData = RX_DATA_INIT();

        printf("socket: %d\n", socket);
        printf("serverAddress: %s\n", serverAddress);
        printf("serverPort: %d\n", serverPort);
        printf("dataLen: %d\n", dataLen);
        printf("data: ");
        for (i=0; i < dataLen; i++)
        {
            printf("%02x", data[i]);
        }
        printf("\n");

        //Assign data to the rx data item
        memcpy(rxData.data, data, dataLen);
        rxData.len = dataLen;

        //Fill in the IP information
        rxData.sourceAddress.address = ParseIPString(serverAddress);
        rxData.sourceAddress.port = serverPort;

        //Add data item to the queue
        enqueueItem(pUbloxModemUdpStruct->pRxQueue, &rxData);
    }
}

typedef enum
{
    UDP_RCV_DECODE_FIND_ECHO,
    UDP_RCV_DECODE_GET_SOCKET,
    UDP_RCV_DECODE_GET_DATA_LENGTH
}
UdpRecvNotificationDecodeState;

void DecodeUbloxUdpNotificationString(UbloxModemUdpStruct *pUbloxModemUdpStruct, char *responseString)
{
    //Recv: +UUSORD: <socket>,<rxlength>
    int len = strlen(responseString);
    UdpRecvNotificationDecodeState currentDecodeState = UDP_RCV_DECODE_FIND_ECHO;
    char argumentString[1024] = "";
    int socket = -1;
    int dataLen = 0;
    int i;
    int argumentStringOffset = 0;

    //Process each character in the response string
    for (i = 0; i < len; i++)
    {
        if (currentDecodeState == UDP_RCV_DECODE_FIND_ECHO)
        {
            if (strMatchFound(argumentString, "+UUSORD:")||strMatchFound(argumentString, "+UUSORF:"))
            {
                currentDecodeState = UDP_RCV_DECODE_GET_SOCKET;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == UDP_RCV_DECODE_GET_SOCKET)
        {
            if (responseString[i] == ',')
            {
                socket = atoi(argumentString);
                currentDecodeState = UDP_RCV_DECODE_GET_DATA_LENGTH;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == UDP_RCV_DECODE_GET_DATA_LENGTH)
        {
            if (responseString[i] == ',')
            {
                dataLen = atoi(argumentString);
                currentDecodeState = UDP_RCV_DECODE_FIND_ECHO;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';

                InitiateModemRead(pUbloxModemUdpStruct->pModemMgmt, dataLen);
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
    }

    //Handles Termination through end of input
    if (currentDecodeState == UDP_RCV_DECODE_GET_DATA_LENGTH)
    {
        dataLen = atoi(argumentString);
        currentDecodeState = UDP_RCV_DECODE_FIND_ECHO;
        argumentStringOffset = 0;
        argumentString[argumentStringOffset] = '\0';

        InitiateModemRead(pUbloxModemUdpStruct->pModemMgmt, dataLen);
    }

    printf("socket: %d\n", socket);
    printf("dataLen: %d\n", dataLen);
}

typedef enum
{
    IP_LOOKUP_DECODE_FIND_ECHO,
    IP_LOOKUP_DECODE_GET_IP,
}
IpLookupNotificationDecodeState;

static int resolvedIP = 0;
static int newLookup = 0;

void DecodeIpLookupString(UbloxModemUdpStruct *pUbloxModemUdpStruct, char *responseString)
{
    //Recv: +UDNSRN: <resolved_ip_address>
    int len = strlen(responseString);
    IpLookupNotificationDecodeState currentDecodeState = IP_LOOKUP_DECODE_GET_IP;
    char argumentString[1024] = "";
    int ip = 0;
    int i;
    int argumentStringOffset = 0;

    //Process each character in the response string
    for (i = 0; i < len; i++)
    {
        if (currentDecodeState == IP_LOOKUP_DECODE_FIND_ECHO)
        {
            if (strMatchFound(argumentString, "+UDNSRN:"))
            {
                currentDecodeState = IP_LOOKUP_DECODE_GET_IP;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
        else if (currentDecodeState == IP_LOOKUP_DECODE_GET_IP)
        {
            if (responseString[i] == ',')
            {
                ip = ParseIPString(argumentString);
                resolvedIP = ip;
                currentDecodeState = IP_LOOKUP_DECODE_FIND_ECHO;
                argumentStringOffset = 0;
                argumentString[argumentStringOffset] = '\0';
                newLookup = 1;
            }
            else
            {
                argumentString[argumentStringOffset++] = responseString[i];
                argumentString[argumentStringOffset] = '\0';
            }
        }
    }

    //Handles Termination through end of input
    if (currentDecodeState == IP_LOOKUP_DECODE_GET_IP)
    {
        ip = ParseIPString(argumentString);
        resolvedIP = ip;
        currentDecodeState = IP_LOOKUP_DECODE_FIND_ECHO;
        argumentStringOffset = 0;
        argumentString[argumentStringOffset] = '\0';
        newLookup = 1;
    }

    printf("ip: %4x\n", ip);
}

ModemCommand removeOldSocketCmd = MODEM_COMMAND_INIT("AT+USOCL=0", &allowErrorModemResponseTable, 5);
ModemCommand createNewSocketCmd = MODEM_COMMAND_INIT("AT+USOCR=17", &defaultModemResponseTable, 5);

void UbloxUdpDataConnectSuccessCallback( void * caller, char * responseString )
{
    //UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;
    //ModemCommand listenCommand = MODEM_COMMAND_INIT("AT+USOLI=0,1200", &defaultModemResponseTable, 10);

    //addModemCommand(pUbloxModemUdpStruct->pModemMgmt, listenCommand);
    UbloxUdpDataConnectFailCallback(caller, responseString);
}

void UbloxUdpDataConnectFailCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;
    char *connectCmd = (char *) malloc(2048);
    ModemCommand connectCommand = MODEM_COMMAND_INIT(connectCmd, &defaultModemResponseTable, 5);
    //int ip = 0;
    //ModemCommand listenCommand = MODEM_COMMAND_INIT("AT+USOLI=0,1200", &defaultModemResponseTable, 10);

    connectCommand.freeCmd = 1;
    sprintf(connectCmd, "AT+USOCO=0,\"%s\",%d", pUbloxModemUdpStruct->hostname, pUbloxModemUdpStruct->port);

    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, removeOldSocketCmd);
    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, createNewSocketCmd);
    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, connectCommand);
    //addModemCommand(pUbloxModemUdpStruct->pModemMgmt, listenCommand);
}

int NameResolveBlocked(UbloxModemUdpStruct *pUbloxModemUdpStruct, char *addressName, int *pResolvedIP)
{
    char *lookupCmd = (char *) malloc(2048);
    ModemCommand lookupIPCommand = MODEM_COMMAND_INIT(lookupCmd, &ipLookupTable, 2);
    int ip = 0;

    lookupIPCommand.freeCmd = 1;
    sprintf(lookupCmd, "AT+UDNSRN=0,\"%s\"", addressName);
    printf("lookupCmd: %s\n", lookupCmd);

    //Clear previous commands
    while (hasModemCommands(pUbloxModemUdpStruct->pModemMgmt))
    {
        mgmt_modem(pUbloxModemUdpStruct->pModemMgmt, pUbloxModemUdpStruct);
        usleep(pUbloxModemUdpStruct->pModemMgmt->msPerTick*1000);
    }

    //Assigning static variables associated with the name lookup call back to get the blocking return to work
    newLookup = 0;

    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, lookupIPCommand);

    //Keep looping until all commands are complete including the ones just added
    while (hasModemCommands(pUbloxModemUdpStruct->pModemMgmt))
    {
        mgmt_modem(pUbloxModemUdpStruct->pModemMgmt, pUbloxModemUdpStruct);
        usleep(pUbloxModemUdpStruct->pModemMgmt->msPerTick*1000);
    }

    if (newLookup)
    {
        ip = resolvedIP;
    }

    *pResolvedIP = ip;

    return ip;
}

void UbloxUdpDataSendCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;

    printf("UbloxUdpDataSendCallback: responseString: %s\n", responseString);
}

void UbloxUdpDataAvailableCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;

    printf("UbloxUdpDataAvailableCallback: responseString: %s\n", responseString);
    DecodeUbloxUdpNotificationString(pUbloxModemUdpStruct, responseString);
}

void UbloxUdpDataRecvCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;

    printf("UbloxUdpDataRecvCallback: responseString: %s\n", responseString);
    DecodeUbloxUdpRecvString(pUbloxModemUdpStruct, responseString);
}

void UbloxDeactivateProfileErrorCallback( void * caller, char * responseString );
void UbloxActivateProfileErrorCallback( void * caller, char * responseString );
void UbloxActivateProfileSuccessCallback( void * caller, char * responseString );

ModemResponseEntry deactivateProfileModemResponseEntries[] =
{
    {"OK", UbloxDeactivateProfileErrorCallback},
    {"ERROR", UbloxDeactivateProfileErrorCallback},
};
ModemResponseTable deactivateModemResponseTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(deactivateProfileModemResponseEntries);

ModemResponseEntry activateProfileModemResponseEntries[] =
{
    {"OK", UbloxActivateProfileSuccessCallback},
    {"ERROR", UbloxActivateProfileErrorCallback},
};
ModemResponseTable activateModemResponseTable = MODEM_RESPONSE_TABLE_ASSIGN_FROM_ARRAY(activateProfileModemResponseEntries);

ModemCommand deactivateProfileCommand = MODEM_COMMAND_INIT("AT+UPSDA=1,4", &deactivateModemResponseTable, 30);
ModemCommand activateProfileCommand = MODEM_COMMAND_INIT("AT+UPSDA=1,3", &activateModemResponseTable, 30);

void UbloxDeactivateProfileErrorCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;

    printf("UbloxDeactivateProfileErrorCallback: responseString: %s\n", responseString);
    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, activateProfileCommand);
}

void UbloxActivateProfileErrorCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;

    printf("UbloxActivateProfileErrorCallback: responseString: %s\n", responseString);
    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, deactivateProfileCommand);
}

void UbloxActivateProfileSuccessCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;
    ModemMgmt *pModemMgmt = pUbloxModemUdpStruct->pModemMgmt;
    char *dataCheckCmd = (char *) malloc(2048);
    ModemCommand dataCheckCommand = MODEM_COMMAND_INIT(dataCheckCmd, &udpConnectProcessTable, 5);

    dataCheckCommand.freeCmd = 1;
    sprintf(dataCheckCmd, "AT+USORF=0,0");

    printf("UbloxActivateProfileSuccessCallback: responseString: %s\n", responseString);

    addModemCommand(pModemMgmt, dataCheckCommand);

    //SetupDataProfile(pUbloxModemUdpStruct);
}

void UbloxPdpDeactivatedCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;

    printf("UbloxPdpDeactivatedCallback: responseString: %s\n", responseString);

    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, removeOldSocketCmd);
    addModemCommand(pUbloxModemUdpStruct->pModemMgmt, deactivateProfileCommand);
}

void UbloxIpLookupCallback( void * caller, char * responseString )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) caller;

    printf("UbloxIpLookupCallback: responseString: %s\n", responseString);
    DecodeIpLookupString(pUbloxModemUdpStruct, responseString);
}

void ublox_init(UbloxModemUdpStruct *pUbloxModemUdpStruct)
{
    //Setup Modem Mgmt
    mgmt_setup(pUbloxModemUdpStruct->pModemMgmt);
    pUbloxModemUdpStruct->pModemMgmt->pModemResponseTable = &asyncModemResponseTable;

    configureUart(pUbloxModemUdpStruct->pModemMgmt->pModemDevice, PROFILE_115200_8N1, 1);

    EnableModem(pUbloxModemUdpStruct->pModemMgmt);
    ConnectCellNetwork(pUbloxModemUdpStruct->pModemMgmt);
    SetupDataProfile(pUbloxModemUdpStruct);

    //Keep looping until all commands are complete including the ones just added
    while (hasModemCommands(pUbloxModemUdpStruct->pModemMgmt))
    {
        mgmt_modem(pUbloxModemUdpStruct->pModemMgmt, pUbloxModemUdpStruct);
        usleep(pUbloxModemUdpStruct->pModemMgmt->msPerTick*1000);
    }
    pUbloxModemUdpStruct->isOpen = 1;
}

void ublox_tick(UbloxModemUdpStruct *pUbloxModemUdpStruct)
{
    mgmt_modem(pUbloxModemUdpStruct->pModemMgmt, pUbloxModemUdpStruct);
}

void ublox_cleanup(UbloxModemUdpStruct *pUbloxModemUdpStruct)
{
    DisconnectCellNetwork(pUbloxModemUdpStruct->pModemMgmt);
    DisableModem(pUbloxModemUdpStruct->pModemMgmt);

    //Keep looping until all commands are complete including the ones just added
    while (hasModemCommands(pUbloxModemUdpStruct->pModemMgmt))
    {
        mgmt_modem(pUbloxModemUdpStruct->pModemMgmt, pUbloxModemUdpStruct);
        usleep(pUbloxModemUdpStruct->pModemMgmt->msPerTick*1000);
    }
    pUbloxModemUdpStruct->isOpen = 0;
}

void ublox_sendto(UbloxModemUdpStruct *pUbloxModemUdpStruct, GenericIP socketAddress, unsigned char *data, int len)
{
    ModemSendTo(pUbloxModemUdpStruct->pModemMgmt, socketAddress, data, len);
}

int ublox_recv(UbloxModemUdpStruct *pUbloxModemUdpStruct, unsigned char *data, int max_len, GenericIP *sourceAddress)
{
    return ModemRecv(pUbloxModemUdpStruct, data, max_len, sourceAddress);
}

int ublox_blocked_namelookup(UbloxModemUdpStruct *pUbloxModemUdpStruct, char *addressName, int *pResolvedIP)
{
    return NameResolveBlocked(pUbloxModemUdpStruct, addressName, pResolvedIP);
}
