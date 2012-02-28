#ifndef MY_HALO_UDP_H
#define MY_HALO_UDP_H

#include "General/types.h"

#define MSG_RECEIVED_ACK  1   //Indicates that a message was received
#define DATA_AVAILABLE    2   //Indicates that there is a new message in this datagram
#define NEW_SESSION       4   //Indicates that a new session is starting and all old state date needs to be cleared
#define SESSION_RESTARTED 8   //Confirms that a session was successfully restarted
#define SESSION_REJECTED  16  //Rejects the proposed session number

typedef struct
{
    uint8  version;
    uint8  status;
    uint16 seqNum;
    uint16 ackSeqNum;
    uint16 payloadLength;
}
MyHaloUdpHeader;

#define MY_HALO_UDP_HEADER_INIT() { \
 .version       = 1, \
 .status        = 0, \
 .seqNum        = 0, \
 .ackSeqNum     = 0, \
 .payloadLength = 0, \
}

typedef struct
{
    MyHaloUdpHeader header;
    uint16 crc;
}
HaloUdpAckPkt;

#define HALO_UDP_ACK_PKT_INIT() { \
 .header       = MY_HALO_UDP_HEADER_INIT(), \
 .crc          = 0, \
}

#endif //MY_HALO_UDP_H
