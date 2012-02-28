#ifndef GENERIC_STATIC_ARRAY_QUEUE_H
#define GENERIC_STATIC_ARRAY_QUEUE_H

typedef struct
{
    unsigned char *array;
    int itemByteSize;
    int numberOfItems;
    int maximumNumberOfItems;
    int arrayHead;
    int arrayTail;
}
GenericStaticArrayQueue;

#define GENERIC_STATIC_ARRAY_QUEUE_INIT(a,b,c) { \
                                   .array = a, \
                                   .itemByteSize = b, \
                                   .maximumNumberOfItems = c, \
                                   .arrayHead = 0, \
                                   .arrayTail = 0, \
                                   .numberOfItems = 0, \
                              }

#define GENERIC_STATIC_ARRAY_QUEUE_DEFAULT_INIT() GENERIC_STATIC_ARRAY_QUEUE_INIT(NULL, 0, 0)

int enqueueItem(GenericStaticArrayQueue *pQueue, void* pItem);
int dequeueItem(GenericStaticArrayQueue *pQueue, int offset);
int peekItem(GenericStaticArrayQueue *pQueue, int offset, void* pItem);
int setItem(GenericStaticArrayQueue *pQueue, int offset, void* pItem);
int isQueueEmpty(GenericStaticArrayQueue *pQueue);
int isQueueFull(GenericStaticArrayQueue *pQueue);
int queueSize(GenericStaticArrayQueue *pQueue);

#endif //GENERIC_STATIC_ARRAY_QUEUE_H
