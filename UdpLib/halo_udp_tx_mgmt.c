//10/24/2011
//CLS (Halo Monitoring)
//Manages holding data pkts desired to be transmitted.
//It remembers what has been sent until an acknowlegement is received from the target
//That it has been sent successfully

#include "tx_buffer.h"
#include "halo_udp_tx_mgmt.h"

#include "../types.h"

int enqueue_tx_packet(HaloUdpTxMgmt *txMgmt, uint8 *data, int len, uint16 seqNum,
                      GenericIP socketAddress)
{
    int success = FAIL;
    TxEntry entry;

    entry.data                = data;
    entry.length              = len;
    entry.seqNum              = seqNum;
    entry.socketAddress       = socketAddress;
    entry.txRetryCount        = 0;

    if (enqueueBuffer(&txMgmt->txMgmtBuffer, entry) == SUCCESS)
    {
        success = SUCCESS;
    }

    return success;
}

int peek_tx_packet(HaloUdpTxMgmt *txMgmt, int offset, uint8 **data, int *len, uint16 *seqNum,
                   GenericIP *socketAddress)
{
    int success = FAIL;
    TxEntry entry;

    if (peekBuffer(&txMgmt->txMgmtBuffer, offset, &entry) == SUCCESS)
    {
        *data                = entry.data;
        *len                 = entry.length;
        *seqNum              = entry.seqNum;
        *socketAddress       = entry.socketAddress;
        success = SUCCESS;
    }

    return success;
}

int dequeue_tx_packet(HaloUdpTxMgmt *txMgmt, GenericIP socketAddress,
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
                if ((entry.socketAddress.address == socketAddress.address)
                &&(entry.socketAddress.port == socketAddress.port)
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

//Tx Retry Management
int reset_tx_retries(HaloUdpTxMgmt *txMgmt, int offset)
{
    int result = FAIL;
    TxEntry entry;

    if (peekBuffer(&txMgmt->txMgmtBuffer, offset, &entry) == SUCCESS)
    {
        entry.txRetryCount = 0;
        result = setBuffer(&txMgmt->txMgmtBuffer, offset, &entry);
    }

    return result;
}

int get_tx_retries(HaloUdpTxMgmt *txMgmt, int offset, int *retries)
{
    int result = FAIL;
    TxEntry entry;

    if (peekBuffer(&txMgmt->txMgmtBuffer, offset, &entry) == SUCCESS)
    {
        *retries = entry.txRetryCount ;
        result = setBuffer(&txMgmt->txMgmtBuffer, offset, &entry);
    }

    return result;
}

int incr_tx_retries(HaloUdpTxMgmt *txMgmt, int offset)
{
    int result = FAIL;
    TxEntry entry;

    if (peekBuffer(&txMgmt->txMgmtBuffer, offset, &entry) == SUCCESS)
    {
        entry.txRetryCount++;
        result = setBuffer(&txMgmt->txMgmtBuffer, offset, &entry);
    }

    return result;
}
