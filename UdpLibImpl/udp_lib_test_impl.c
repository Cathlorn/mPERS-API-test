//UDP Communications Library
//CLS (Halo Monitoring)
//ATTENTION: Code requires UDP IPv4. Will quit if not supported
//TO DO: Add support for IPv6 UDP
//TODO: Create an implementation that retreives bytes from a static pool either of PosixUDP Data structures or from generic bytes.
#include <stdio.h>

#include "UdpLib/udp_lib.h"

#define BUFFSIZE 320

//Function Prototypes
void udp_recv_tick(UdpCommStruct *commStruct);

//These functions must be implemented to get the data needed for test
extern int test_recv(uint8 *data, int max_len);
extern int test_send(uint8 *data, int len);

void udp_init ( UdpCommStruct *commStruct )
{
}

void udp_cleanup(UdpCommStruct *commStruct)
{
}

int udp_sendto(UdpCommStruct *commStruct, uint8 *data, int len,
               GenericIP socketIP)
{
    int sent = -1;

    sent = test_send(data,len);

    if (commStruct->debug)
        printf("Sent %d bytes\n", sent);

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

    received = test_recv(data,max_len);

    if ((commStruct->debug)&&(received > 0))
        printf("Rcvd %d bytes\n", received);

    commStruct->recvData = data;
    commStruct->recvLength = received;

    //TEST: Putting dummy address for the return addresses
    if (commStruct->actAsServer)
    {
        commStruct->clientIP.address = 1;
        commStruct->clientIP.port    = 2;
    }
    else
    {
        commStruct->socketIP.address = 2;
        commStruct->socketIP.port    = 2;
    }

    return received;
}

void udp_tick(UdpCommStruct *commStruct)
{
    //Handle Transmit

    //Handle Receive
    udp_recv_tick(commStruct);
}

void udp_recv_tick(UdpCommStruct *commStruct)
{
    unsigned char buffer[BUFFSIZE];
    UdpRecvArgs args;

    if (udp_recv(commStruct, buffer, sizeof(buffer)) > 0) //new data
    {
        args.data   = commStruct->recvData;
        args.length = commStruct->recvLength;
        args.commStruct = commStruct;

        commStruct->dataReceived((void *) &args);
    }
}
