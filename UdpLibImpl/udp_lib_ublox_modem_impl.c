//UDP Communications Library port for Ublox Cellular Modem Modules
//CLS (Halo Monitoring)
//ATTENTION: Code requires UDP IPv4. Will quit if not supported
//TODO: Add support for IPv6 UDP

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "General/types.h"
#include "UdpLib/udp_lib.h"
#include "Ublox/ublox_modem_udp.h"

#define BUFFSIZE 1024

#define MAX_MODEM_COMMANDS  40
#define MAX_RX_DATA_ENTRIES 60
#define MS_SLEEP_PER_TICK   10

//Assigns Modem File
UartControl atModemDevice = UART_CONTROL_INIT("/dev/ttyUSB1");

//Initializes Command Queue
ModemCommand commandArray[MAX_MODEM_COMMANDS] = {[0 ... MAX_MODEM_COMMANDS-1] = MODEM_COMMAND_DEFAULT_INIT()};
ModemCmdQueue txCmdQueue = MODEM_CMD_QUEUE_INIT((unsigned char *) commandArray, sizeof(ModemCommand), MAX_MODEM_COMMANDS);

//Initializes Data Queue
RxData rxDataEntries[MAX_RX_DATA_ENTRIES] = {[0 ... MAX_RX_DATA_ENTRIES-1] = RX_DATA_INIT()};
GenericStaticArrayQueue rxDataQueue = GENERIC_STATIC_ARRAY_QUEUE_INIT((unsigned char *) rxDataEntries, sizeof(RxData), MAX_RX_DATA_ENTRIES);

//Initializes Manage interface
ModemMgmt modemMgmt = MODEM_MGMT_INIT(&atModemDevice, &txCmdQueue, NULL, MS_SLEEP_PER_TICK);

//Populates master data structure with information
UbloxModemUdpStruct ubloxModemUdpStruct = UBLOX_MODEM_UDP_STRUCT_INIT(&modemMgmt, &rxDataQueue);

void udp_recv_tick(UdpCommStruct *commStruct);

void udp_init ( UdpCommStruct *commStruct )
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) &ubloxModemUdpStruct;
    //For now, making this so that it is single instance implementation.
    //TODO: Make more generic this wrapper in future
    commStruct->udpSocketDataPtr = pUbloxModemUdpStruct;

    commStruct->socketIP.port = atoi(commStruct->port);
    pUbloxModemUdpStruct->hostname = commStruct->hostname;
    pUbloxModemUdpStruct->port = atoi(commStruct->port);

    ublox_init(pUbloxModemUdpStruct);

    //Handles server connections
    if (commStruct->actAsServer)
    {
        //Server
        //Bind connection
        //Read POSIX socket to populate Generic IP Address
        //commStruct->socketIP.address = 0;
        commStruct->socketIP.address = 0x7f000001; //127.0.0.1

        //TO DO: Add more server stuff here
    }
    else
    {
        //Client
        //Resolve the name of the server
        //Read POSIX socket to populate Generic IP Address
        ublox_blocked_namelookup(pUbloxModemUdpStruct, commStruct->hostname, (int *) &commStruct->socketIP.address);
        pUbloxModemUdpStruct->hostnameIp = commStruct->socketIP.address;
    }
}

int udp_isOpen(UdpCommStruct *commStruct)
{
    int isOpen = 0;
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) commStruct->udpSocketDataPtr;

    if (pUbloxModemUdpStruct)
    {
        isOpen = pUbloxModemUdpStruct->isOpen;
    }

    return isOpen;
}

void udp_cleanup(UdpCommStruct *commStruct)
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) commStruct->udpSocketDataPtr;

    if (pUbloxModemUdpStruct)
    {
        ublox_cleanup(pUbloxModemUdpStruct);
        if (commStruct->debug)
            printf ( "closing udp connection\n" );
    }
}

int udp_sendto(UdpCommStruct *commStruct, uint8 *data, int len,
               GenericIP socketIP)
{
    int sent = -1;
    UdpEventData udpEventData;
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) commStruct->udpSocketDataPtr;

    if (pUbloxModemUdpStruct)
    {
        commStruct->sendData   = data;
        commStruct->sendLength = len;

        sent = len; //Debug assume all data is sent
        ublox_sendto(pUbloxModemUdpStruct, socketIP, data, len);

        if (sent > 0)
        {
            udpEventData.length = len;
            udpEventData.data = data;
            udpEventData.commStruct = commStruct;
            if (commStruct->dataSent)
                commStruct->dataSent((void *) &udpEventData);
        }

        if (commStruct->debug)
            printf("Sent %d bytes\n", sent);
    }

    return sent;
}

int udp_send(UdpCommStruct *commStruct, uint8 *data, int len)
{
    GenericIP socketAddr;

    if (commStruct->actAsServer)
    {
        socketAddr = commStruct->clientIP;
    }
    else
    {
        socketAddr = commStruct->socketIP;
    }

    return udp_sendto(commStruct, data, len, socketAddr);
}

int udp_recv(UdpCommStruct *commStruct, uint8 *data, int max_len)
{
    int received = -1;
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) commStruct->udpSocketDataPtr;
    GenericIP socketAddr = GENERIC_IP_INIT();

    if (pUbloxModemUdpStruct)
    {
        received = ublox_recv(pUbloxModemUdpStruct, data, max_len, &socketAddr);

        if(received > 0)
        {
            commStruct->recvData = data;
            commStruct->recvLength = received;

            //Read POSIX socket to populate Generic IP Address
            commStruct->rcvIP = socketAddr;

            if (commStruct->actAsServer)
            {
                commStruct->clientIP = socketAddr;
            }
            else
            {
                commStruct->socketIP = socketAddr;
            }

            if (commStruct->debug)
                printf("Rcvd %d bytes\n", received);
        }
    }

    return received;
}

void udp_tick(UdpCommStruct *commStruct)
{
    UbloxModemUdpStruct *pUbloxModemUdpStruct = (UbloxModemUdpStruct *) commStruct->udpSocketDataPtr;

    ublox_tick(pUbloxModemUdpStruct);
    //Handle Transmit
    //Handle Receive
    udp_recv_tick(commStruct);
}

void udp_recv_tick(UdpCommStruct *commStruct)
{
    unsigned char buffer[BUFFSIZE];
    UdpEventData udpEventData;

    if (udp_recv(commStruct, buffer, sizeof(buffer)) > 0) //new data
    {
        udpEventData.data   = commStruct->recvData;
        udpEventData.length = commStruct->recvLength;
        udpEventData.commStruct = commStruct;

        commStruct->dataReceived((void *) &udpEventData);
    }
}
