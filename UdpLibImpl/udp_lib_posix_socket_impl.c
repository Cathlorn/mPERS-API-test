//UDP Communications Library
//CLS (Halo Monitoring)
//ATTENTION: Code requires UDP IPv4. Will quit if not supported
//TO DO: Add support for IPv6 UDP
//TODO: Create an implementation that retreives bytes from a static pool either of PosixUDP Data structures or from generic bytes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <assert.h>

#include "../types.h"
#include "UdpLib/udp_lib.h"
#include "udp_lib_posix_socket_impl.h"

#define BUFFSIZE 320
#define POSIX_DATA_ARRAY_SIZE 2
#define USE_POSIX_DYNAMIC_ALLOC 0

#if !USE_POSIX_DYNAMIC_ALLOC
static PosixUdpData posixDataArray[POSIX_DATA_ARRAY_SIZE];
static int posixDataArrayCount = 0;
#endif

//Made really dumb. You can only allocate, can't free
PosixUdpData * getPosixDataPointer(void)
{
    PosixUdpData * posixUdpDataPtr = NULL;

#if USE_POSIX_DYNAMIC_ALLOC
    posixUdpDataPtr = (PosixUdpData *) malloc(sizeof(PosixUdpData));
#else
    assert(posixDataArrayCount < POSIX_DATA_ARRAY_SIZE);
    posixUdpDataPtr = &posixDataArray[posixDataArrayCount];
    posixDataArrayCount++;
#endif

    return posixUdpDataPtr;
}

void freePosixDataPointer(PosixUdpData * posixUdpDataPtr)
{
#if USE_POSIX_DYNAMIC_ALLOC
    free(posixUdpDataPtr);
#else
    //Do Nothing
    //TODO: Create an implementation that retreives bytes from a static pool either of PosixUDP Data structures
    //or from generic bytes.
#endif
}

//Function Prototypes
void udp_recv_tick(UdpCommStruct *commStruct);

void udp_init ( UdpCommStruct *commStruct )
{
    int sock_flags;
    struct addrinfo *result = NULL;
    struct addrinfo *addrReadPtr = NULL;
    int error = -1;
    int udpAddrFound = 0;
    PosixUdpData *posixDataPtr = NULL;

    posixDataPtr = getPosixDataPointer();
    commStruct->udpSocketDataPtr = posixDataPtr;

    posixDataPtr->rcvAddrPtr = NULL;
    posixDataPtr->rcvAddrLenPtr = NULL;
    posixDataPtr->clientAddr.sin_family = AF_INET;  // Internet/IP
    posixDataPtr->clientAddr.sin_port = htons ( 0 ); // client port
    posixDataPtr->clientAddr.sin_addr.s_addr = 0;  // client IP address
    posixDataPtr->clientAddr_len = sizeof(struct sockaddr_in); //This MUST be initialized for receiving to work

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
        if ( ( posixDataPtr->sock = socket ( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
        {
            //ErrorHandler ( "Failed to create socket" );
            printf ( "Failed to create socket\n" );
            assert(0);
        }

        posixDataPtr->socketAddress.sin_family = AF_INET;                  // Internet/IP

        commStruct->socketIP.port = atoi(commStruct->port);
        posixDataPtr->socketAddress.sin_port = htons ( commStruct->socketIP.port ); // server port

        posixDataPtr->socketAddressLength = sizeof ( struct sockaddr_in );
        //Handles server connections
        if (commStruct->actAsServer)
        {
            //Server
            //Bind connection
            posixDataPtr->socketAddress.sin_addr.s_addr = 0;  // IP address
            if ( bind ( posixDataPtr->sock, ( struct sockaddr * ) &posixDataPtr->socketAddress, posixDataPtr->socketAddressLength ) )
            {
                printf( "Failed to bind to port with server\n" );
                assert(0);
            }
        }
        else
        {
            //Client

            posixDataPtr->socketAddress.sin_addr.s_addr = ((struct sockaddr_in *) addrReadPtr->ai_addr)->sin_addr.s_addr;  // IP address
        }

        //Read POSIX socket to populate Generic IP Address
        commStruct->socketIP.address = posixDataPtr->socketAddress.sin_addr.s_addr;

        // put client socket into nonblocking mode
        sock_flags = fcntl ( posixDataPtr->sock, F_GETFL, 0 );

        fcntl ( posixDataPtr->sock, F_SETFL, sock_flags | O_NONBLOCK );
    }
    else
    {
        //Report error
        printf("Error! IPv4 Socket not first name lookup entry. Transfer will FAIL!\n");
        assert(0);
    }

    freeaddrinfo(result);
}

int udp_isOpen(UdpCommStruct *commStruct)
{
    int isOpen = 0;
    PosixUdpData *posixDataPtr = (PosixUdpData *) commStruct->udpSocketDataPtr;

    if(posixDataPtr)
    {
        isOpen = ( posixDataPtr->sock >= 0 );
    }

    return isOpen;
}

void udp_cleanup(UdpCommStruct *commStruct)
{
    PosixUdpData *posixDataPtr = (PosixUdpData *) commStruct->udpSocketDataPtr;

    close ( posixDataPtr->sock );
    posixDataPtr->sock = -1;

    //TO DO: Make static implementation where dynamic cleanup is not needed

    //Dynamic Memory Allocation cleanup
    if (commStruct->udpSocketDataPtr)
    {
        freePosixDataPointer(commStruct->udpSocketDataPtr);
        commStruct->udpSocketDataPtr = NULL;
    }

    if (commStruct->debug)
        printf ( "closing app.......\n" );
}

int udp_sendto(UdpCommStruct *commStruct, uint8 *data, int len,
               GenericIP socketIP)
{
    int sent = -1;
    struct sockaddr_in socketAddress;
    UdpEventData udpEventData;
    socklen_t socketAddressLength;
    PosixUdpData *posixDataPtr = (PosixUdpData *) commStruct->udpSocketDataPtr;

    socketAddress.sin_family = AF_INET;  // Internet/IP
    socketAddress.sin_port = htons ( socketIP.port ); // client port
    socketAddress.sin_addr.s_addr = socketIP.address;  // client IP address
    socketAddressLength = sizeof(struct sockaddr_in); //This MUST be initialized for receiving to work

    commStruct->sendData   = data;
    commStruct->sendLength = len;

    sent = sendto ( posixDataPtr->sock, data, len, 0,
                    (struct sockaddr *) &socketAddress,
                    socketAddressLength );

    if(sent > 0)
    {
        udpEventData.length = len;
        udpEventData.data = data;
        udpEventData.commStruct = commStruct;
        if(commStruct->dataSent)
            commStruct->dataSent((void *) &udpEventData);
    }

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
    struct sockaddr_in *sockAddrPtr = NULL;
    socklen_t *sockAddrLenPtr = NULL;
    PosixUdpData *posixDataPtr = (PosixUdpData *) commStruct->udpSocketDataPtr;
    GenericIP socketAddr;

    if (commStruct->actAsServer)
    {
        sockAddrPtr = ( struct sockaddr_in * ) &posixDataPtr->clientAddr;
        sockAddrLenPtr = (socklen_t *) &posixDataPtr->clientAddr_len;
    }
    else
    {
        sockAddrPtr = ( struct sockaddr_in * ) &posixDataPtr->socketAddress;
        sockAddrLenPtr = (socklen_t *) &posixDataPtr->socketAddressLength;
    }

    assert(sockAddrPtr);
    assert(sockAddrLenPtr);

    *sockAddrLenPtr = sizeof(struct sockaddr_in);  //Makes sure the maximum size can return back is passed in (needed for recvfrom to work correctly)
    received = recvfrom ( posixDataPtr->sock, data, max_len, 0,
                          (struct sockaddr *) sockAddrPtr,
                          sockAddrLenPtr);


    commStruct->recvData = data;
    commStruct->recvLength = received;
    posixDataPtr->rcvAddrPtr = sockAddrPtr;
    posixDataPtr->rcvAddrLenPtr = sockAddrLenPtr;

    //Read POSIX socket to populate Generic IP Address
    socketAddr.address = sockAddrPtr->sin_addr.s_addr;
    socketAddr.port = ntohs(sockAddrPtr->sin_port);
    commStruct->rcvIP = socketAddr;

    if (commStruct->actAsServer)
    {
        commStruct->clientIP = socketAddr;
    }
    else
    {
        commStruct->socketIP = socketAddr;
    }

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
    UdpEventData udpEventData;

    if (udp_recv(commStruct, buffer, sizeof(buffer)) > 0) //new data
    {
        udpEventData.data   = commStruct->recvData;
        udpEventData.length = commStruct->recvLength;
        udpEventData.commStruct = commStruct;

        commStruct->dataReceived((void *) &udpEventData);
    }
}
