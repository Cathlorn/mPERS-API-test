#ifndef UDP_LIB_H
#define UDP_LIB_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "../types.h"

typedef void ( *NotifierFunction ) ( void * );

typedef struct
{
    char hostname[1024];
    char port[1024];
    struct sockaddr_in socketAddress;
    socklen_t socketAddressLength;
    int sock;
    uint8 debug;
    uint8 actAsServer;

    uint8 *sendData;
    int sendLength;

    uint8 *recvData;
    int recvLength;

    struct sockaddr_in clientAddr;
    socklen_t clientAddr_len;

    struct sockaddr_in *rcvAddrPtr;
    socklen_t *rcvAddrLenPtr;

    //Event Notifiers for when data is processed
    //NotifierFunction tickCalled;
    NotifierFunction dataSent;
    NotifierFunction dataReceived;
}
UdpCommStruct;


typedef struct
{
    uint8  *data;
    uint16 length;
    UdpCommStruct *commStruct;
}
UdpRecvArgs;


//Setup the port
void udp_init(UdpCommStruct *commStruct);

//Handles tx and rx mangement
void udp_tick(UdpCommStruct *commStruct);

void udp_close(UdpCommStruct *commStruct);

int udp_send(UdpCommStruct *commStruct, uint8 *data, int len);
int udp_sendto(UdpCommStruct *commStruct, uint8 *data, int len,
               struct sockaddr_in *sockAddrPtr, socklen_t sockAddrLen);
int udp_recv(UdpCommStruct *commStruct, uint8 *data, int max_len);

#endif //UDP_LIB_H
