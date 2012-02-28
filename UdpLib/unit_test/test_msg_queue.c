//NOTE: This implementation does NOT do any memory allocation for the data only returns pointers
//It is the responsibility of the calling functions to manage memory if needed.

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "test_msg_queue.h"
#include "General/types.h"

#define MAX_QUEUE_SIZE 2000//20

typedef struct
{
    uint8 *data;
    int len;
}
QueueItem;

#define QUEUE_ITEM_INIT() { \
 .data       = NULL, \
 .len        = 0, \
}

typedef struct
{
    QueueItem items[MAX_QUEUE_SIZE];
    int headIndex;
    int tailIndex;
    int queueSize;
}
Queue;

#define QUEUE_INIT() { \
 .items       = { [0..MAX_QUEUE_SIZE-1] QUEUE_ITEM_INIT()}, \
 .headIndex   = 0, \
 .tailIndex   = 0, \
 .queueSize   = 0, \
}

#define QUEUE_CLR(p) { \
 p->headIndex   = 0; \
 p->tailIndex   = 0; \
 p->queueSize   = 0; \
}

//Generic Queue Handlers

//returns BOOL success / fail
int enqueue(Queue *queue, void *data, int len)
{
    int success = 0;

    if(queue->queueSize < MAX_QUEUE_SIZE)
    {
        //Add to end
        queue->items[queue->tailIndex].data = data;
        queue->items[queue->tailIndex].len = len;

        //Advance end
        queue->tailIndex = ((queue->tailIndex + MAX_QUEUE_SIZE + 1) % MAX_QUEUE_SIZE);
        queue->queueSize++;
        success = 1;
    }

    return success;
}

int dequeue(Queue *queue)
{
    int success = 0;

    if(queue->queueSize > 0)
    {
        //Advance front
        queue->headIndex = ((queue->headIndex + MAX_QUEUE_SIZE + 1) % MAX_QUEUE_SIZE);
        queue->queueSize--;

        success = 1;
    }

    return success;
}

void clear(Queue *queue)
{
    QUEUE_CLR(queue);
}

int queueSize(Queue *queue)
{
    return queue->queueSize;
}

//returns BOOL success / fail
int queueEmpty(Queue *queue)
{
    return (queue->queueSize > 0);
}

//returns BOOL success / fail
int peek(Queue *queue,void **data, int *len)
{
    int success = 0;

    if(queue->queueSize > 0)
    {
        //Report from front
        *data = queue->items[queue->headIndex].data;
        *len = queue->items[queue->headIndex].len;

        success = 1;
    }

    return success;
}

//Specific Msg Queue Impl

static Queue msgQueue;

//NOTE: decided to have the actual message queue do dynamic memory allocation to make things easier
//A static impl substitue could be used instead

//returns BOOL success / fail
int enqueueMsg(void *data, int len)
{
    uint8 *tempDataPtr = NULL;

    //Allocate Memory
    tempDataPtr = malloc(len);

    assert(tempDataPtr);

    memcpy(tempDataPtr,data, len);

    return enqueue(&msgQueue, tempDataPtr, len);
}

int dequeueMsg()
{
    int success = 0;
    uint8 *tempDataPtr = NULL;
    int tempDataLen = 0;

    if(peek(&msgQueue, (void **) &tempDataPtr, &tempDataLen)) //Make sure can read
    {
        success = dequeue(&msgQueue); //Make sure dequeue works
        //Release Memory
        if(success)
        {
            free(tempDataPtr);
        }
    }

    return success;
}

void clearMsgs()
{
    //clear(&msgQueue);
    //Using these methods to clear everything quickly
    while(msgsAvailable())
    {
        dequeueMsg();
    }
}

int msgsAvailable()
{
    return queueSize(&msgQueue);
}

//returns BOOL success / fail
int hasMsgs()
{
    return queueEmpty(&msgQueue);
}

//returns BOOL success / fail
int peekMsg(void **data, int *len)
{
    return peek(&msgQueue, data, len);
}
