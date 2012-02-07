//MyHalo UDP Protocol Wrapper

#ifndef HALO_UDP_COMM_H
#define HALO_UDP_COMM_H

#include "mpers/HaloMessage.h"
#include "types.h"
#include "GenericIP.h"
#include "halo_udp_stats.h"

//Configuration

//Suggested settings ( 3 seconds between rexmit, and 10 retries) => assumes 2ms per tick;
//2 minutes of inactivity before dropping a session
#define TICKS_PER_SECOND            500
#define TICKS_PER_MIN               (60 * TICKS_PER_SECOND)
#define TICKS_PER_REXMIT            (3 * TICKS_PER_SECOND)
#define TICKS_PER_INACTIVE_SESSION  (120 * TICKS_PER_SECOND)
#define MAX_REXMIT                  10
#define MAX_CONCURRENT_CONNNECTIONS 2

//User Notification

//NOTE: Might want to have code that handles these be conditional compiled to reduce footprint when
//compiled for a device
typedef struct
{
    uint8 loopback : 1; //Send back to client data it receives
    uint8 badCrc : 1;   //Corrupt the CRCs of all data coming in
    uint8 neverTxDrop : 1; //Makes msgs that normally would be dropped when transmitting stay in queue
    uint8 neverAck : 1; //Make it so that acknowledgements never happen
    uint8 duplicateTx : 1;
    uint8 outOfSeqTx : 1;
    uint8 spottyRx;
}
HaloUdpCommDbg;

#define HALO_UDP_COMM_DBG_INIT() { \
 .loopback    = 0, \
 .badCrc      = 0, \
 .neverTxDrop = 0, \
 .neverAck    = 0, \
 .duplicateTx = 0, \
 .outOfSeqTx  = 0, \
 .spottyRx  = 0, \
}

typedef struct
{
    uint8 debug;
    uint8 actAsServer;
    const char * hostname;  //Report NULL if user server mode
    const char * port;
    NotifierFunction  msg_tx_sent;
    NotifierFunction  msg_tx_dropped;
    NotifierFunction  msg_rx_received;
    HaloUdpCommDbg dbgTestCtrls;
}
HaloUdpUserData;

#define HALO_UDP_USER_DATA_INIT() { \
 .debug = 0, \
 .actAsServer = 0, \
 .hostname = "", \
 .port = "", \
 .msg_tx_sent = NULL, \
 .msg_tx_dropped = NULL, \
 .msg_rx_received = NULL, \
 .dbgTestCtrls = HALO_UDP_COMM_DBG_INIT(), \
}

typedef struct
{
    int dataLength;
    uint8 *data;
    GenericIP socketAddress;
}
HaloUdpEventData;

#define HALO_UDP_EVENT_DATA_INIT() { \
 .dataLength = 0, \
 .data = NULL, \
 .socketAddress = GENERIC_IP_INIT(), \
}

//Operation
void halo_msg_init(HaloUdpUserData *userData);
void halo_msg_new_session(int sessionIndex);
//int halo_msg_send(const HaloMessage *msg);
//int halo_msg_sendto(const HaloMessage *msg,
//                    GenericIP socketAddress);
//int halo_msg_send_to_index(const HaloMessage *msg, int sessionIndex);
int halo_msg_send(const void *data, const int len);
int halo_msg_sendto(const void *data, const int len,
                    GenericIP socketAddress);
int halo_msg_send_to_index(const void *data, const int len, int sessionIndex);
int halo_msg_session_count(void);
void halo_msg_report_session(int offset);
void halo_msg_tick(void);
void halo_msg_cleanup(void);

void halo_msg_report_stats();
void halo_msg_reset_stats();
HaloUdpStats get_halo_msg_stats();

//Halo UDP Protocol Debug Testing Control functions
HaloUdpCommDbg get_halo_udp_comm_dbg(void);
void set_halo_udp_comm_dbg(HaloUdpCommDbg dbgTestCtrls);

#endif //HALO_UDP_COMM_H
