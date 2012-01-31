//UDP Interface Layer

#ifndef UDP_LIB_H
#define UDP_LIB_H

#include "types.h"
#include "GenericIP.h"

#define UDP_COMM_STR_LENGTH 1024

typedef struct
{
    char hostname[UDP_COMM_STR_LENGTH];
    char port[UDP_COMM_STR_LENGTH];
    uint8 debug;
    uint8 actAsServer;

    uint8 *sendData;
    int sendLength;

    uint8 *recvData;
    int recvLength;

    GenericIP socketIP;
    GenericIP clientIP;
    GenericIP rcvIP;

    //Pointer to implementation-specific data
    void *udpSocketDataPtr;

    //Event Notifiers for when data is processed
    NotifierFunction dataSent;
    NotifierFunction dataReceived;
}
UdpCommStruct;

#define UDP_COMM_STRUCT_INIT() { \
 .hostname         = "", \
 .port             = "", \
 .debug            = 0, \
 .actAsServer      = 0, \
 .sendData         = NULL, \
 .sendLength       = 0, \
 .recvData         = NULL, \
 .recvLength       = 0, \
 .socketIP         = GENERIC_IP_INIT(), \
 .clientIP         = GENERIC_IP_INIT(), \
 .rcvIP            = GENERIC_IP_INIT(), \
 .udpSocketDataPtr = NULL, \
 .dataSent         = NULL, \
 .dataReceived     = NULL, \
}

typedef struct
{
    uint8  *data;
    uint16 length;
    UdpCommStruct *commStruct;
}
UdpEventData;

//Setup the port
void udp_init(UdpCommStruct *commStruct);

//Handles tx and rx mangement
void udp_tick(UdpCommStruct *commStruct);

void udp_cleanup(UdpCommStruct *commStruct);

int udp_send(UdpCommStruct *commStruct, uint8 *data, int len);
int udp_sendto(UdpCommStruct *commStruct, uint8 *data, int len,
               GenericIP socketAddress);

int udp_recv(UdpCommStruct *commStruct, uint8 *data, int max_len);

#endif //UDP_LIB_H
