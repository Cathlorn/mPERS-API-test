//10/24/2011
//CLS (Halo Monitoring)
//Generic Buffer pool that returns points to different statically allocated segments of memory
//Pool implements a circular queue from a static array

#include "types.h"
#include "tx_buffer_static.h"

//Configuration
#define TX_BUFFER_SIZE 1492//10
#define TX_BUFFER_DATA_BLOCK_SIZE 256

typedef struct
{
    uint8 used;
    uint8 data[TX_BUFFER_DATA_BLOCK_SIZE];
}
BufferEntry;

#define BUFFER_ENTRY_INIT() { \
 .used  = 0, \
 .data = {0}, \
}

typedef struct
{
    int numberOfAllocatedEntries;
    BufferEntry entries[TX_BUFFER_SIZE];
}
BufferArray;

static BufferArray bufferArray = { \
                                   .numberOfAllocatedEntries = 0, \
                                   .entries       = { [0 ... TX_BUFFER_SIZE-1] BUFFER_ENTRY_INIT()}, \
                                 };

int getBuffer(int requested_length, void **data, int *max_length)
{
    int result = FAIL;
    int i;

    //Space on the Buffer
    if ((bufferArray.numberOfAllocatedEntries < TX_BUFFER_SIZE)&&(requested_length <= TX_BUFFER_DATA_BLOCK_SIZE))
    {
        //Find first free spot
        for (i=0; i< TX_BUFFER_SIZE; i++)
        {
            if (!bufferArray.entries[i].used)
            {
                //Match found
                //Point to the correct buffer
                *data = bufferArray.entries[i].data;
                //Report the defined maximum length
                *max_length = TX_BUFFER_DATA_BLOCK_SIZE;

                bufferArray.entries[i].used = 1;
                //Update position
                bufferArray.numberOfAllocatedEntries++;

                //Update result
                result = SUCCESS;
                break;
            }
        }
    }

    return result;
}

int freeBuffer(void *data)
{
    int result = FAIL;
    int i;

    if (bufferArray.numberOfAllocatedEntries > 0)
    {
        //Find pointer that has been freed
        for (i=0; i< TX_BUFFER_SIZE; i++)
        {
            if (data == bufferArray.entries[i].data)
            {
                //Match found
                bufferArray.entries[i].used = 0;
                //Update position
                bufferArray.numberOfAllocatedEntries--;

                //Update result
                result = SUCCESS;
                break;
            }
        }
    }

    return result;
}
