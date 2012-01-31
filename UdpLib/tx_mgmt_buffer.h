#ifndef TX_MGMT_BUFFER_H
#define TX_MGMT_BUFFER_H

#include "types.h"
#include "GenericIP.h"

//Configuration
#define TX_MGMT_BUFFER_SIZE 1492//7

//Define TX_MGMT_BUFFER_H_DEBUG to compile test code
//#define TX_MGMT_BUFFER_H_DEBUG //uncomment line compile/use debug code

typedef struct
{
    uint8 *data;
    int length;
    uint16 seqNum;
    GenericIP socketAddress;
    uint8 txRetryCount;
}
TxEntry;

typedef struct
{
    int arrayHead;
    int arrayTail;
    int numberOfAllocatedEntries;
    TxEntry entries[TX_MGMT_BUFFER_SIZE];
}
TxMgmtBuffer;

#define TX_MGMT_BUFFER_INIT() { \
                                   .arrayHead = 0, \
                                   .arrayTail = 0, \
                                   .numberOfAllocatedEntries = 0, \
                              }

int enqueueBuffer(TxMgmtBuffer *txMgmtBuffer, TxEntry txEntry);
int dequeueBuffer(TxMgmtBuffer *txMgmtBuffer, int offset);
int peekBuffer(TxMgmtBuffer *txMgmtBuffer, int offset, TxEntry *txEntry);
int setBuffer(TxMgmtBuffer *txMgmtBuffer, int offset, TxEntry *txEntry);
int isBufferEmpty(TxMgmtBuffer *txMgmtBuffer);
int isBufferFull(TxMgmtBuffer *txMgmtBuffer);
int bufferSize(TxMgmtBuffer *txMgmtBuffer);

#endif //TX_MGMT_BUFFER_H
