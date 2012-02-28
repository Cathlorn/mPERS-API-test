#include <stdio.h>
#include <string.h>

#include "GenericStaticArrayQueue.h"

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif

int enqueueItem(GenericStaticArrayQueue *pQueue, void* pItem)
{
    int result = FAIL;

    //Space on the Buffer
    if (pQueue->numberOfItems < pQueue->maximumNumberOfItems)
    {
        //Store the data
        //pQueue->array[pQueue->arrayTail] = txEntry;

        //Copy the item into the array
        memcpy(&pQueue->array[pQueue->itemByteSize * pQueue->arrayTail], pItem, pQueue->itemByteSize);

        //Update position
        pQueue->arrayTail = (pQueue->arrayTail + 1) % pQueue->maximumNumberOfItems; //Increment Postion
        pQueue->numberOfItems++;

        //Update result
        result = SUCCESS;
    }

    return result;
}

int dequeueItem(GenericStaticArrayQueue *pQueue, int offset)
{
    int result = FAIL;

    if ((pQueue->numberOfItems > 0)&&(offset < pQueue->numberOfItems))
    {
        int currentIndex;
        int nextPosition;
        int shiftsRemaining = 0;

        //Shift data to remove entry
        currentIndex = (pQueue->arrayHead + offset) % pQueue->maximumNumberOfItems;
        nextPosition = (currentIndex - 1 + pQueue->maximumNumberOfItems) % pQueue->maximumNumberOfItems;
        shiftsRemaining = offset;
        while (shiftsRemaining)
        {
            //Copy data backwards from the empty slot at 'offset'
            pQueue->array[currentIndex] = pQueue->array[nextPosition];
            currentIndex = nextPosition;
            nextPosition = (currentIndex - 1 + pQueue->maximumNumberOfItems) % pQueue->maximumNumberOfItems;
            shiftsRemaining--;
        }

        //Update position
        pQueue->arrayHead = (pQueue->arrayHead + 1 + pQueue->maximumNumberOfItems) % pQueue->maximumNumberOfItems; //Increment Postion
        pQueue->numberOfItems--;

        //Update result
        result = SUCCESS;
    }

    return result;
}

int peekItem(GenericStaticArrayQueue *pQueue, int offset, void* pItem)
{
    int result = FAIL;

    if ((pQueue->numberOfItems > 0)&&(offset < pQueue->numberOfItems))
    {
        int entryPosition;

        entryPosition = (((pQueue->arrayHead) + offset) % pQueue->maximumNumberOfItems); //Increment Postion
        //Return queue value
        //*txEntry  = pQueue->array[entryPosition];
        //pItem  = &pQueue->array[pQueue->itemByteSize * entryPosition];
        memcpy(pItem, &pQueue->array[pQueue->itemByteSize * entryPosition], pQueue->itemByteSize);

        //Update result
        result = SUCCESS;
    }

    return result;
}

int setItem(GenericStaticArrayQueue *pQueue, int offset, void* pItem)
{
    int result = FAIL;

    if ((pQueue->numberOfItems > 0)&&(offset < pQueue->numberOfItems))
    {
        int entryPosition;

        entryPosition = (pQueue->arrayHead + offset) % pQueue->maximumNumberOfItems; //Increment Postion

        //pQueue->array[entryPosition] = *txEntry;
        memcpy(&pQueue->array[pQueue->itemByteSize * entryPosition], pItem, pQueue->itemByteSize);

        //Update result
        result = SUCCESS;
    }

    return result;
}

int isQueueEmpty(GenericStaticArrayQueue *pQueue)
{
    return (pQueue->numberOfItems <= 0);
}

int isQueueFull(GenericStaticArrayQueue *pQueue)
{
    return (pQueue->numberOfItems >= pQueue->maximumNumberOfItems);
}

int queueSize(GenericStaticArrayQueue *pQueue)
{
    return pQueue->numberOfItems;
}
