//UDP Communications Library
//CLS (Halo Monitoring)
//ATTENTION: Code requires UDP IPv4. Will quit if not supported
//TO DO: Add support for IPv6 UDP

#include "udp_lib.h"

#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <assert.h>

#define BUFFSIZE 320

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>

#include "../types.h"

//Function Prototypes
void udp_recv_tick(UdpCommStruct *commStruct);

void udp_init ( UdpCommStruct *commStruct )
{
    int sock_flags;
    struct addrinfo *result = NULL;
    struct addrinfo *addrReadPtr = NULL;
    int error = -1;
    int udpAddrFound = 0;


    commStruct->rcvAddrPtr = NULL;
    commStruct->rcvAddrLenPtr = NULL;
    commStruct->clientAddr.sin_family = AF_INET;  // Internet/IP
    commStruct->clientAddr.sin_port = htons ( 0 ); // client port
    commStruct->clientAddr.sin_addr.s_addr = 0;  // client IP address
    commStruct->clientAddr_len = sizeof(struct sockaddr_in); //This MUST be initialized for receiving to work

    error = getaddrinfo(commStruct->hostname, commStruct->port, NULL, &result);

    assert(!error);
    assert(result);

    addrReadPtr = result;

    //Find the IPv4 UDP Entry
    while (addrReadPtr)
    {
        if ((addrReadPtr->ai_family == PF_INET)&&(addrReadPtr->ai_protocol == IPPROTO_UDP))
        {
            udpAddrFound = 1;
            break;
        }

        //Advance list
        addrReadPtr = addrReadPtr->ai_next;
    }

    if (udpAddrFound) //Handles UDP IPv4
    {

        // Create the UDP socket
        if ( ( commStruct->sock = socket ( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        {
            //ErrorHandler ( "Failed to create socket" );
            printf ( "Failed to create socket\n" );
            assert(0);
        }

        commStruct->socketAddress.sin_family = AF_INET;                  // Internet/IP

        commStruct->socketAddress.sin_port = htons ( atoi(commStruct->port) ); // server port

        commStruct->socketAddressLength = sizeof ( struct sockaddr_in );
        //Handles server connections
        if (commStruct->actAsServer)
        {
            //Server
            //Bind connection
            commStruct->socketAddress.sin_addr.s_addr = 0;  // IP address
            if ( bind ( commStruct->sock, ( struct sockaddr * ) &commStruct->socketAddress, commStruct->socketAddressLength ) )
            {
                printf( "Failed to bind to port with server\n" );
                assert(0);
            }
        }
        else
        {
            //Client
            commStruct->socketAddress.sin_addr.s_addr = ((struct sockaddr_in *) addrReadPtr->ai_addr)->sin_addr.s_addr;  // IP address
        }

        // put client socket into nonblocking mode
        sock_flags = fcntl ( commStruct->sock, F_GETFL, 0 );

        fcntl ( commStruct->sock, F_SETFL, sock_flags | O_NONBLOCK );
    }
    else
    {
        //Report error
        printf("Error! IPv4 Socket not first name lookup entry. Transfer will FAIL!\n");
        assert(0);
    }

    freeaddrinfo(result);
}

void udp_close(UdpCommStruct *commStruct)
{
    close ( commStruct->sock );
    commStruct->sock = -1;

    if (commStruct->debug)
        printf ( "closing app.......\n" );
}

int udp_sendto(UdpCommStruct *commStruct, uint8 *data, int len,
               struct sockaddr_in *sockAddrPtr, socklen_t sockAddrLen)
{
    int sent = -1;

    assert(sockAddrPtr);

    sent = sendto ( commStruct->sock, data, len, 0,
                    (struct sockaddr *) sockAddrPtr,
                    sockAddrLen );

    if (commStruct->debug)
        printf("Sent %d bytes\n", sent);

    return sent;
}

int udp_send(UdpCommStruct *commStruct, uint8 *data, int len)
{
    struct sockaddr_in *sockAddrPtr = NULL;
    socklen_t *sockAddrLenPtr = NULL;

    if (commStruct->actAsServer)
    {
        sockAddrPtr = ( struct sockaddr_in * ) &commStruct->clientAddr;
        sockAddrLenPtr = (socklen_t *) &commStruct->clientAddr_len;
    }
    else
    {
        sockAddrPtr = ( struct sockaddr_in * ) &commStruct->socketAddress;
        sockAddrLenPtr = (socklen_t *) &commStruct->socketAddressLength;
    }

    return udp_sendto(commStruct, data, len, sockAddrPtr, *sockAddrLenPtr);
}

int udp_recv(UdpCommStruct *commStruct, uint8 *data, int max_len)
{
    int received = -1;
    struct sockaddr_in *sockAddrPtr = NULL;
    socklen_t *sockAddrLenPtr = NULL;

    if (commStruct->actAsServer)
    {
        sockAddrPtr = ( struct sockaddr_in * ) &commStruct->clientAddr;
        sockAddrLenPtr = (socklen_t *) &commStruct->clientAddr_len;
    }
    else
    {
        sockAddrPtr = ( struct sockaddr_in * ) &commStruct->socketAddress;
        sockAddrLenPtr = (socklen_t *) &commStruct->socketAddressLength;
    }

    assert(sockAddrPtr);
    assert(sockAddrLenPtr);

    *sockAddrLenPtr = sizeof(struct sockaddr_in);  //Makes sure the maximum size can return back is passed in (needed for recvfrom to work correctly)
    received = recvfrom ( commStruct->sock, data, max_len, 0,
                          (struct sockaddr *) sockAddrPtr,
                          sockAddrLenPtr);


    commStruct->recvData = data;
    commStruct->recvLength = received;
    commStruct->rcvAddrPtr = sockAddrPtr;
    commStruct->rcvAddrLenPtr = sockAddrLenPtr;

    if ((commStruct->debug)&&(received > 0))
        printf("Rcvd %d bytes\n", received);

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
