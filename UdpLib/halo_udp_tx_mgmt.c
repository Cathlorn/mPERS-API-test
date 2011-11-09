//10/24/2011
//CLS (Halo Monitoring)
//Manages holding data pkts desired to be transmitted.
//It remembers what has been sent until an acknowlegement is received from the target
//That it has been sent successfully

#include "tx_buffer.h"
#include "halo_udp_tx_mgmt.h"

#include "../types.h"

int enqueue_tx_packet(HaloUdpTxMgmt *txMgmt, uint8 *data, int len, uint16 seqNum,
                      struct sockaddr_in socketAddress, socklen_t socketAddressLength)
{
    int success = FAIL;
    TxEntry entry;

    entry.data                = data;
    entry.length              = len;
    entry.seqNum              = seqNum;
    entry.socketAddress       = socketAddress;
    entry.socketAddressLength = socketAddressLength;

    if (enqueueBuffer(&txMgmt->txMgmtBuffer, entry) == SUCCESS)
    {
        success = SUCCESS;
    }

    return success;
}

int peek_tx_packet(HaloUdpTxMgmt *txMgmt, int offset, uint8 **data, int *len, uint16 *seqNum,
                   struct sockaddr_in *socketAddress, socklen_t *socketAddressLength)
{
    int success = FAIL;
    TxEntry entry;

    if (peekBuffer(&txMgmt->txMgmtBuffer, offset, &entry) == SUCCESS)
    {
        *data                = entry.data;
        *len                 = entry.length;
        *seqNum              = entry.seqNum;
        *socketAddress       = entry.socketAddress;
        *socketAddressLength = entry.socketAddressLength;
        success = SUCCESS;
    }

    return success;
}

int dequeue_tx_packet(HaloUdpTxMgmt *txMgmt, struct sockaddr_in socketAddress, socklen_t socketAddressLength,
                      uint16 seqNum)
{
    int success = FAIL;

    if (!isBufferEmpty(&txMgmt->txMgmtBuffer))
    {
        TxEntry entry;
        int i;
        int mgmtBufferSize;

        mgmtBufferSize = bufferSize(&txMgmt->txMgmtBuffer);
        for ( i = 0; i < mgmtBufferSize; i++)
        {
            if (peekBuffer(&txMgmt->txMgmtBuffer, i, &entry) == SUCCESS)
            {
                //Make sure that the correct packet is being removed from the tx queue
                if ((entry.socketAddress.sin_addr.s_addr == socketAddress.sin_addr.s_addr)
                &&(entry.socketAddress.sin_port == socketAddress.sin_port)
                &&(entry.seqNum == seqNum))
                {
                    success = dequeueBuffer(&txMgmt->txMgmtBuffer, i);
                    break;
                }
            }
        }
    }

    return success;
}

int pending_tx_packet(HaloUdpTxMgmt *txMgmt)
{
    return !isBufferEmpty(&txMgmt->txMgmtBuffer);
}

int tx_packet_queue_size(HaloUdpTxMgmt *txMgmt)
{
    return bufferSize(&txMgmt->txMgmtBuffer);
}
