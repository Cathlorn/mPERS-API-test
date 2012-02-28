#ifndef UBLOX_MODEM_UDP_H
#define UBLOX_MODEM_UDP_H

#include "Modem/at_mgmt.h"
#include "General/GenericStaticArrayQueue.h"
#include "UdpLib/GenericIP.h"

//Receive Data processing

#define RX_DATA_BUFFER_SIZE 2048
typedef struct
{
    int max_len;
    int len;
    unsigned char data[RX_DATA_BUFFER_SIZE];
    GenericIP sourceAddress;
}
RxData;

#define RX_DATA_INIT() {\
  .max_len = RX_DATA_BUFFER_SIZE, \
  .len = 0, \
  .data = { [ 0 ... RX_DATA_BUFFER_SIZE-1 ] 0 }, \
  .sourceAddress = GENERIC_IP_INIT(), \
}

//Master Data structure for UDP Mgmt
#define MAX_HOSTNAME_LENGTH 1024
typedef struct
{
    ModemMgmt *pModemMgmt;
    GenericStaticArrayQueue *pRxQueue; //Uses an array of RxData type
    int isOpen;
    char *hostname;
    int hostnameIp;
    int port;
    int maxHostNameLength;
}
UbloxModemUdpStruct;

#define UBLOX_MODEM_UDP_STRUCT_INIT(a,b) { \
                                   .pModemMgmt = a, \
                                   .pRxQueue = b, \
                                   .isOpen = 0, \
                                   .hostname = NULL, \
                                   .hostnameIp = 0, \
                                   .port = 0, \
                                   .maxHostNameLength = MAX_HOSTNAME_LENGTH, \
                              }

#define UBLOX_MODEM_UDP_STRUCT_DEFAULT_INIT() UBLOX_MODEM_UDP_STRUCT_INIT(NULL,NULL)

void ublox_init(UbloxModemUdpStruct *pUbloxModemUdpStruct);
void ublox_tick(UbloxModemUdpStruct *pUbloxModemUdpStruct);
void ublox_cleanup(UbloxModemUdpStruct *pUbloxModemUdpStruct);

void ublox_sendto(UbloxModemUdpStruct *pUbloxModemUdpStruct, GenericIP socketAddress, unsigned char *data, int len);
int ublox_recv(UbloxModemUdpStruct *pUbloxModemUdpStruct, unsigned char *data, int max_len, GenericIP *sourceAddress);

int ublox_blocked_namelookup(UbloxModemUdpStruct *pUbloxModemUdpStruct, char *addressName, int *pResolvedIP);

#endif //UBLOX_MODEM_UDP_H
