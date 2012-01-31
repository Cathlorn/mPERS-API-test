#ifndef HALO_UDP_TX_MGMT_H
#define HALO_UDP_TX_MGMT_H

#include "types.h"
#include "GenericIP.h"
#include "tx_mgmt_buffer.h"

typedef struct
{
    TxMgmtBuffer txMgmtBuffer;
}
HaloUdpTxMgmt;

#define HALO_UDP_TX_MGMT_INIT() { \
 .txMgmtBuffer = TX_MGMT_BUFFER_INIT(), \
}

int enqueue_tx_packet(HaloUdpTxMgmt *txMgmt, uint8 *data, int len, uint16 seqNum,
                      GenericIP socketAddress);

int peek_tx_packet(HaloUdpTxMgmt *txMgmt, int offset, uint8 **data, int *len, uint16 *seqNum,
                   GenericIP *socketAddress);

int dequeue_tx_packet(HaloUdpTxMgmt *txMgmt, GenericIP socketAddress,
                      uint16 seqNum);

//Tx Retry Management
int reset_tx_retries(HaloUdpTxMgmt *txMgmt, int offset);
int get_tx_retries(HaloUdpTxMgmt *txMgmt, int offset, int *retries);
int incr_tx_retries(HaloUdpTxMgmt *txMgmt, int offset);

int pending_tx_packet(HaloUdpTxMgmt *txMgmt);

int tx_packet_queue_size(HaloUdpTxMgmt *txMgmt);

#endif //HALO_UDP_TX_MGMT_H
