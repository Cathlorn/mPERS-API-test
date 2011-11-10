#include <stdio.h>
#include "tx_mgmt_buffer.h"

int enqueueBuffer(TxMgmtBuffer *txMgmtBuffer, TxEntry txEntry)
{
    int result = FAIL;

    //Space on the Buffer
    if (txMgmtBuffer->numberOfAllocatedEntries < TX_MGMT_BUFFER_SIZE)
    {
        //Store the data
        txMgmtBuffer->entries[txMgmtBuffer->arrayTail] = txEntry;

        //Update position
        txMgmtBuffer->arrayTail = (txMgmtBuffer->arrayTail + 1) % TX_MGMT_BUFFER_SIZE; //Increment Postion
        txMgmtBuffer->numberOfAllocatedEntries++;

        //Update result
        result = SUCCESS;
    }

    return result;
}

int dequeueBuffer(TxMgmtBuffer *txMgmtBuffer, int offset)
{
    int result = FAIL;

    if ((txMgmtBuffer->numberOfAllocatedEntries > 0)&&(offset < txMgmtBuffer->numberOfAllocatedEntries))
    {
        int currentIndex;
        int nextPosition;

        //Shift data to remove entry
        currentIndex = (txMgmtBuffer->arrayHead + offset) % TX_MGMT_BUFFER_SIZE; //Increment Postion;
        nextPosition = (currentIndex + 1) % TX_MGMT_BUFFER_SIZE;
        while (nextPosition != txMgmtBuffer->arrayTail) //Copy until you reach the end (Tail points to the next available slot so don't copy that
        {
            txMgmtBuffer->entries[currentIndex] = txMgmtBuffer->entries[nextPosition];
            currentIndex = nextPosition;
            nextPosition = (currentIndex + 1) % TX_MGMT_BUFFER_SIZE;
        }

        //Update position
        txMgmtBuffer->arrayTail = (txMgmtBuffer->arrayTail - 1 + TX_MGMT_BUFFER_SIZE) % TX_MGMT_BUFFER_SIZE; //Increment Postion
        txMgmtBuffer->numberOfAllocatedEntries--;

        //Update result
        result = SUCCESS;
    }

    return result;
}

int peekBuffer(TxMgmtBuffer *txMgmtBuffer, int offset, TxEntry *txEntry)
{
    int result = FAIL;

    if ((txMgmtBuffer->numberOfAllocatedEntries > 0)&&(offset < txMgmtBuffer->numberOfAllocatedEntries))
    {
        int entryPosition;

        entryPosition = (txMgmtBuffer->arrayHead + offset) % TX_MGMT_BUFFER_SIZE; //Increment Postion
        //Return queue value
        *txEntry   = txMgmtBuffer->entries[entryPosition];

        //Update result
        result = SUCCESS;
    }

    return result;
}

int setBuffer(TxMgmtBuffer *txMgmtBuffer, int offset, TxEntry *txEntry)
{
    int result = FAIL;

    if ((txMgmtBuffer->numberOfAllocatedEntries > 0)&&(offset < txMgmtBuffer->numberOfAllocatedEntries))
    {
        int entryPosition;

        entryPosition = (txMgmtBuffer->arrayHead + offset) % TX_MGMT_BUFFER_SIZE; //Increment Postion

        txMgmtBuffer->entries[entryPosition] = *txEntry;

        //Update result
        result = SUCCESS;
    }

    return result;
}

int isBufferEmpty(TxMgmtBuffer *txMgmtBuffer)
{
    return (txMgmtBuffer->numberOfAllocatedEntries <= 0);
}

int isBufferFull(TxMgmtBuffer *txMgmtBuffer)
{
    return (txMgmtBuffer->numberOfAllocatedEntries >= TX_MGMT_BUFFER_SIZE);
}

int bufferSize(TxMgmtBuffer *txMgmtBuffer)
{
    return txMgmtBuffer->numberOfAllocatedEntries;
}


//Test Code for structure
#ifdef TX_MGMT_BUFFER_H_DEBUG


//Tests and makes sure that buffer actually works
void bufferTest()
{
    TxMgmtBuffer txMgmtBuffer = TX_MGMT_BUFFER_INIT();
    const char data1[] = "Yippe";
    const char data2[] = "Ki Yay!";
    TxEntry entry1;
    TxEntry entry2;
    TxEntry tmpEntry;
    int i;
    int bufferCount;

    entry1.data = data1;
    entry1.length = sizeof(data1);
    entry1.sessionIndex = 0;
    entry1.seqNum = 0;

    entry2.data = data2;
    entry2.length = sizeof(data2);
    entry2.sessionIndex = 1;
    entry2.seqNum = 1;

    if (isBufferEmpty(&txMgmtBuffer))
    {
        printf("Buffer is empty!\n");
    }
    else
    {
        printf("Buffer is NOT empty!\n");
    }


    enqueueBuffer(&txMgmtBuffer, entry1);
    enqueueBuffer(&txMgmtBuffer, entry2);

    if (isBufferEmpty(&txMgmtBuffer))
    {
        printf("Buffer is empty!\n");
    }
    else
    {
        printf("Buffer is NOT empty!\n");
    }

    //Print out
    bufferCount = bufferSize(&txMgmtBuffer);
    printf("Buffer Size: %d\n", bufferCount);

    for (i=0; i < bufferCount; i++)
    {
        if (peekBuffer(&txMgmtBuffer, i, &tmpEntry) == SUCCESS)
        {
            printf("data        : %s\n", tmpEntry.data);
            printf("length      : %d\n", tmpEntry.length);
            printf("seqNum      : %d\n", tmpEntry.seqNum);
            printf("sessionIndex: %d\n", tmpEntry.sessionIndex);
            printf("\n");
        }
        else
        {
            printf("peekBuffer failed at index %s!\n", i);
            printf("\n");
        }
    }

    /*    for (i=0; i < bufferCount; i++)
        {
            printf("Buffer Size: %d\n", bufferCount);
            if (dequeueBuffer(&txMgmtBuffer, 0) == SUCCESS)
            {
                printf("dequeueBuffer succeeded!\n");
            }
            else
            {
                printf("dequeueBuffer failed!\n");
            }
        } */

    printf("Buffer Size: %d\n", bufferSize(&txMgmtBuffer));
    if (dequeueBuffer(&txMgmtBuffer, 1) == SUCCESS)
    {
        printf("dequeueBuffer succeeded!\n");
        if (peekBuffer(&txMgmtBuffer, 0, &tmpEntry) == SUCCESS)
        {
            printf("data        : %s\n", tmpEntry.data);
            printf("length      : %d\n", tmpEntry.length);
            printf("seqNum      : %d\n", tmpEntry.seqNum);
            printf("sessionIndex: %d\n", tmpEntry.sessionIndex);
            printf("\n");
        }
        else
        {
            printf("peekBuffer failed at index %s!\n", i);
            printf("\n");
        }
    }
    else
    {
        printf("dequeueBuffer failed!\n");
    }

    printf("Buffer Size: %d\n", bufferSize(&txMgmtBuffer));
    if (dequeueBuffer(&txMgmtBuffer, 0) == SUCCESS)
    {
        printf("dequeueBuffer succeeded!\n");
    }
    else
    {
        printf("dequeueBuffer failed!\n");
    }

    printf("Buffer Size: %d\n", bufferSize(&txMgmtBuffer));

    if (isBufferEmpty(&txMgmtBuffer))
    {
        printf("Buffer is empty!\n");
    }
    else
    {
        printf("Buffer is NOT empty!\n");
    }

    if (isBufferFull(&txMgmtBuffer))
    {
        printf("Buffer is full!\n");
    }
    else
    {
        printf("Buffer is NOT full!\n");
    }

}


#endif
